/*
 ============================================================================
 Name        : server_time.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <limits.h>
#include <mqueue.h>
#include <time.h>
#include <poll.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>


#include "apisyslog.h"
#include "utilstools_listtailqueue.h"
#include "utilstools_network.h"

#include "libmsg_srvtimer.h"


//static sDataThreadCtx_t g_Context_signaldate = {0} ;

ListQ_t g_List_clienttimer  = {0};
ListQ_t g_List_Register     = {0};
ListQ_t g_List_UnRegister   = {0};

sThreadDataCtx_t    g_ThreadDataCtx_Timer = {0};

static int servicetime_cbfcnt_svc_getdate(
        const sRequestServer_t  *a_pRequest,
        sResponse_t *a_pResponse)

{
    int result = 0;
    char msgbuffer[APISYSLOG_MSG_SIZE] = {0};

    (void)a_pRequest;
    sGetdateResponse_t  *pResponseData      = (sGetdateResponse_t *)&a_pResponse->data;

    a_pResponse->header.datasize =
            sizeof(sHeader_t) + sizeof(pResponseData->timespesc);

    result = clock_gettime(CLOCK_MONOTONIC_RAW,
            &pResponseData->timespesc);

    snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
            " : date=%ld.%09ld result=%d\n",
            pResponseData->timespesc.tv_sec,
            pResponseData->timespesc.tv_nsec,
            result);

    a_pResponse->header.result = result;

    TRACE_LOG(msgbuffer);

    return result;
}

static int servicetime_cbfcnt_svc_timer(
        const sRequestServer_t  *a_pRequestServer,
        sResponse_t *a_pResponse)

{
    int     result              = 0;
    char    host[NI_MAXHOST]    = {0};
    char    service[NAME_MAX]   = {0}; // NI_MAXSERV
    int     nbitem              = 0;
    sRequestServer_t    *pRequestServer = 0;
    ListQ_item_t        *pItem          = 0;
    sTimerRequest_t     *pTimerRequest  = 0;

    //duplicate request to store
    pRequestServer = libsmg_dupRequestServer((sRequestServer_t*)a_pRequestServer);

    pTimerRequest = (sTimerRequest_t*)&pRequestServer->request.data;

#if 0
    result = getnameinfo((struct sockaddr*)&a_pRequestServer->peer_addr,
            a_pRequestServer->peer_addr_len,
            host,sizeof(host),
            service,   sizeof(service),
            NI_NUMERICHOST | NI_NUMERICSERV);

    if( 0 != result )
        result = -1;
#endif

    tq_lock(&g_List_Register);

    pItem = tq_insertTail(&g_List_Register);

    pItem->pData = (void*)pRequestServer;

    nbitem = tq_size(&g_List_Register);

    tq_unlock(&g_List_Register);

    // send signal to thread
    result = pthread_kill(g_ThreadDataCtx_Timer.pthreadID,SIGUSR1);

    // send signal to thread


    //*******************************************************
    //* response only ok , client can acivate thread reception notification
    //*******************************************************
    a_pResponse->header.result = 0;
    a_pResponse->header.datasize = sizeof(a_pResponse->header);

    TRACE_DBG1(": service= %s host= %s date=%ld.%ld nbitem=%d result=%d",
            service,host, pTimerRequest->timespesc.tv_sec,
            pTimerRequest->timespesc.tv_nsec,nbitem,result);

    return result;
}

int servicetime_removeitemlist(ListQ_t *a_pList,ListQ_item_t *a_pItem)
{
    int result = 0;
    tq_lock(a_pList);

    tq_removeItemList(a_pList,a_pItem);

    sRequestServer_t    *pRequestServer  = (sRequestServer_t *)a_pItem->pData;
    struct itimerspec time_isp = {0};

    // reset timer
    result = timer_settime(pRequestServer->timerid,0,&time_isp,0);

    tq_destroyItem(a_pItem,1);

    tq_unlock(a_pList);

    TRACE_DBG1("timer_settime(%d)=%d size=%d",pRequestServer->timerid,result,
            tq_size(a_pList) );

    return 0;
}

//********************************************
//*
//********************************************
static void servicetime_funcThread_timerCB(__sigval_t a_sigval)
{
    int result = 0;
    int socket_client = -1;
    sResponse_t response = {0};
//    char    localhost[NI_MAXHOST]  = {0};
    char    localservice[NAME_MAX] = {0}; // NI_MAXSERV
    ListQ_item_t        *pItem              = a_sigval.sival_ptr;
    sRequestServer_t    *pRequestServer  = (sRequestServer_t *)pItem->pData;
    sTimerResponse_t   *pTimerResponse  = {0};


    char        msgbuffer[APISYSLOG_MSG_SIZE]   = {0};


    result = getnameinfo( (struct sockaddr*)&pRequestServer->peer_addr,
            pRequestServer->peer_addr_len,
            0,0,    //            localhost,       sizeof(localhost),
            localservice,    sizeof(localservice),NI_NUMERICSERV); // NI_NUMERICHOST |
    if (0 == result)
    {
        TRACE_DBG1("_1_ : getnameinfo()=%d service=%s  peer_addr_len=%d",result, localservice, pRequestServer->peer_addr_len);
    }
    else
    {
        TRACE_DBG1(" _12_: getnameinfo()=%d %s", result,gai_strerror(result));
        //result = 0;
    }

    //***********************************************************
    //                open client
    //***********************************************************
    TRACE_DBG1("_2_ localservice=%s result=%d",localservice,result);

    result = net_openConnect(localservice,&socket_client);

    if( 0 != result)
    {
        snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                " _3_ : net_openConnect(%.50s) result=%d %s \n",
                localservice,result , strerror(result));
        TRACE_ERR(msgbuffer);

       servicetime_removeitemlist(&g_List_clienttimer,pItem);
    }

    if( 0 == result )
    {
        TRACE_DBG1("_4_ result=0x%X",result);

        pTimerResponse = (sTimerResponse_t*)&response.data;

        clock_gettime(CLOCK_MONOTONIC_RAW,&pTimerResponse->timespesc);

        response.header.datasize = sizeof(sHeader_t) + sizeof(struct timespec);

        // send msg to server
        result = sendto(socket_client,&response,response.header.datasize,0,
                (struct sockaddr*)&pRequestServer->peer_addr,
                pRequestServer->peer_addr_len);


        TRACE_DBG2("_5_sendto()=%d size=%d errno=%u %s\n",
                result,
                response.header.datasize,
                errno,strerror(errno));

        if ( -1 == result)
        {
            snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                    "error sendto(%s) result=%d errno=%d %s \n",
                    localservice,result , errno,strerror(errno));
            TRACE_ERR(msgbuffer);
        }
    }


    close(socket_client);

    TRACE_DBG4("_6_ : result=%d client=%s date=%ld.%lld",
            result,localservice,
            pTimerResponse->timespesc.tv_sec,
            pTimerResponse->timespesc.tv_nsec);

}

static void * servicetime_threadFunction_timer(void * a_pArg)
{
    int result = 0;
    sThreadDataCtx_t *pContext = (sThreadDataCtx_t *) a_pArg;

    sigset_t        sigset      = {0};
    siginfo_t       siginfo     = {0};
    struct sigevent sigevent    = {0};
    struct sigaction sSigaction = {0};

    timer_t             timerid   = 0;
    struct itimerspec   its       = {0};
    ListQ_item_t        *pItem = 0;

    sRequestServer_t    *pRequestServer       = 0;
    sTimerRequest_t     *pTimerRequest  = 0;

    //configure signal
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGUSR1);
    sigaddset(&sigset, SIGALRM);

    result = pthread_sigmask(SIG_BLOCK, &sigset, NULL);

    do
    {
        memset(&siginfo,0,sizeof(siginfo));
        result = sigwaitinfo(&sigset, &siginfo);
        TRACE_DBG2(": _1_ sigwaitinfo() = %d %d %s",result,errno,strerror(errno));

        if( SIGUSR1 == result ) // add client
        {

            tq_lock(&g_List_Register);
            pItem = tq_removeHeadList(&g_List_Register);
            tq_unlock(&g_List_Register);

            pRequestServer  = (sRequestServer_t*)pItem->pData;
            pTimerRequest   = (sTimerRequest_t*)&pRequestServer->request.data;


            tq_lock(&g_List_clienttimer);
            tq_insertItemTail(&g_List_clienttimer,pItem);
            tq_unlock(&g_List_clienttimer);



            // create timer
            sigevent.sigev_notify_function = servicetime_funcThread_timerCB;
            sigevent.sigev_notify = SIGEV_THREAD; // SIGEV_SIGNAL;
            sigevent.sigev_signo = SIGALRM;
            //sigevent.sigev_value.sival_int = pRequestServer->timerid;
            sigevent.sigev_value.sival_ptr = (void*)pItem;

            result = timer_create(CLOCK_MONOTONIC, &sigevent, &pRequestServer->timerid);

            TRACE_DBG2(": _2_ timer_create()=%d %d %s pRequest=%#p",
                    result,errno,strerror(errno),pItem);

            its.it_value.tv_sec     = pTimerRequest->timespesc.tv_sec;
            its.it_value.tv_nsec    = pTimerRequest->timespesc.tv_nsec;
            its.it_interval.tv_sec  = its.it_value.tv_sec;
            its.it_interval.tv_nsec = its.it_value.tv_nsec;

            result = timer_settime(pRequestServer->timerid, 0, &its, NULL);
            TRACE_DBG2(": _3_ timer_create()=%d timeout=%ld.%lld %d %s",
                    result,
                    pTimerRequest->timespesc.tv_sec,
                    pTimerRequest->timespesc.tv_nsec,
                    errno,strerror(errno));
        }
        else if( SIGUSR2 == result ) // remove client
        {
            TRACE_DBG2(": _4_ sigwaitinfo = SIGUSR2");
        }
        else if( SIGALRM == result ) //
        {
            TRACE_DBG2(": _4_ sigwaitinfo = SIGALRM");
        }
        else
        {
            //error
        }

    }while(1);

    return (void*)0;
}


//*********************************************************
//*
//*********************************************************
int main(void)
{
    int result = 0;

    apisyslog_init("");
    sleep(1);

    tq_init(&g_List_clienttimer,sizeof(sRequestServer_t));
    tq_init(&g_List_Register,sizeof(sRequestServer_t));


    result = libmsg_srvtimer_srv_register_svc_getdate(SRVTIMER_GETDATE,servicetime_cbfcnt_svc_getdate);


    result =  pthread_create(&g_ThreadDataCtx_Timer.pthreadID,
            NULL,
            &servicetime_threadFunction_timer,
            (void*)&g_ThreadDataCtx_Timer);


    result = libmsg_srvtimer_srv_register_svc_timer(SRVTIMER_TIMER,servicetime_cbfcnt_svc_timer);


    result = libmsg_srvtimer_srv_wait();

    return result;
}

