/*
 * server_time.c
 *
 *  Created on: 4 juin 2020
 *      Author: bruno
 */
#define _GNU_SOURCE

#include <stdio.h>

#include <fcntl.h>           /* Pour les constantes O_* */
#include <sys/stat.h>        /* Pour les constantes des modes */
#include <mqueue.h>

#include <errno.h>
#include <string.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>

// #define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <limits.h>

#include "apisyslog.h"

#include "libmsg_srvtime.h"


// client  : /process.id.svc
// server  : /process.svc
//

static sDataThreadCtx_t g_ThreadCtx_srv_svc_getdate          = {0};

static sDataThreadCtx_t g_ThreadCtx_srv_svc_Signaldate       = {0};
static sDataThreadCtx_t g_ThreadCtx_cli_SignaldateNotify     = {0};
static sDataThreadCtx_t g_ThreadCtx_srv_signaldateNotify     = {0};



int libmsg_srvtime_cli_getdate1( _IN_ const char* a_UniqID, _OUT_ double *a_Date)
{

}
//************************************************************
//  client side
//      _OUT_ double *a_pDate : buffer data output
//      return:
//          SUCCESS    0
//          EINVAL          22  Invalid argument
//************************************************************
int libmsg_srvtime_cli_getdate( _IN_ const char* a_UniqID, _OUT_ double *a_Date)
{
    int     result                  = 0;
    char msgbuffer[APISYSLOG_MSG_SIZE] = {0};

    sDataService_t      dataService;

//    sRequest_t          *pRequest  = 0;
//    sResponse_t         *pResponse = 0;
    sGetdateResponse_t  *pResponse_getdate  = 0;

    memset(&dataService,0,sizeof(dataService));

    result = getMqClientname(a_UniqID,SVC_GETDATE,dataService.request.filenameClient);

    if( 0 == result )
    {
        result = libmsg_cli_getdata( SERVER_TIME_GETDATE,
                &dataService);
    }

    if( 0 == result )
    {
        pResponse_getdate  = (sGetdateResponse_t  *)&dataService.response.data ;
        *a_Date = getDateDouble(pResponse_getdate->timespesc);

        snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                " : date=%ld.%09ld *date=%.9f result=%d\n",
                pResponse_getdate->timespesc.tv_sec,
                pResponse_getdate->timespesc.tv_nsec,
                *a_Date,
                result);
        TRACE_LOG(msgbuffer);
    }

    return result;
}

//************************************************************
//*
//************************************************************
int libmsg_srvtime_srv_register_svc_getdate(libmsg_pFunctCB_t a_pFunctCB)
{
    int result = 0;

    //*****************************
    // prepare data thread
    //*****************************

    g_ThreadCtx_srv_svc_getdate.dataService.pFunctCB = a_pFunctCB;

    strncpy(g_ThreadCtx_srv_svc_getdate.dataService.filenameServer,
            SERVER_TIME_GETDATE,
            sizeof(g_ThreadCtx_srv_svc_getdate.dataService.filenameServer)-1);

    result = libmsg_srv_register_svc(&g_ThreadCtx_srv_svc_getdate);

    return result;
}
//************************************************************
//*
//************************************************************
int libmsg_srvtime_srv_register_svc_signal(libmsg_pFunctCB_t a_pFunctCB)
{
    int result = 0;

    //*****************************
    // incomming message
    //*****************************

    g_ThreadCtx_srv_signaldateNotify.dataService.pFunctCB = a_pFunctCB;

    strncpy(g_ThreadCtx_srv_signaldateNotify.dataService.filenameServer,
            SERVER_TIME_SIGNALDATE,
            sizeof(g_ThreadCtx_srv_signaldateNotify.dataService.filenameServer)-1);

    result = libmsg_srv_register_svc(&g_ThreadCtx_srv_signaldateNotify);

    //*****************************
    // sending notification
    //*****************************
    result = libmsg_srvtime_register_signal(&g_ThreadCtx_srv_signaldateNotify);


    return result;
}
//************************************************************
//  client side
//      _OUT_ double *a_pDate : buffer data output
//      return:
//          SUCCESS    0
//          EINVAL          22  Invalid argument
//************************************************************
int libmsg_srvtime_cli_signaldate( const char* a_UniqID,
                                const double a_Date,
                                libmsg_pFunctCB_t a_pFunctCB)
{
    int     result                  = 0;
    char msgbuffer[APISYSLOG_MSG_SIZE] = {0};

    sDataService_t      dataServiceRequest = {0};

    struct sSignaldateRequest *pRequestData = 0;

    result = getMqClientname(a_UniqID,SVC_SIGNALDATE,dataServiceRequest.request.filenameClient);

    if( 0 == result )
    {
        pRequestData = (struct sSignaldateRequest*)dataServiceRequest.request.data;

        time_cnv_double_to_ts(a_Date,&pRequestData->timespesc);

        result = libmsg_cli_getdata( SERVER_TIME_SIGNALDATE,&dataServiceRequest);
    }

    if( 0 == result )
    {
        snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                " : result = %d ",dataServiceRequest.response.header.result);
        TRACE_LOG(msgbuffer);
    }

    //****************************************************************
    g_ThreadCtx_cli_SignaldateNotify.dataService.pFunctCB = a_pFunctCB;


//    strncpy(g_ThreadCtx_cli_SignaldateNotify.dataService.filenameServer,
//            dataServiceRequest.request.filenameClient,
//            sizeof(g_ThreadCtx_cli_SignaldateNotify.dataService.filenameServer)-1);


    strncpy(g_ThreadCtx_cli_SignaldateNotify.dataService.request.filenameClient,
            dataServiceRequest.request.filenameClient,
            sizeof(g_ThreadCtx_cli_SignaldateNotify.dataService.request.filenameClient)-1);

    result = libmsg_cli_register_svc(&g_ThreadCtx_cli_SignaldateNotify);


    return result;
}

//************************************************************
//*  return     : 0 if ok
//              : -1 allready registred
//              : > 0 error code
//************************************************************
int libmsg_srvtime_srv_signaldate_addClientNotify(const sRequest_t *a_pRequest)
{
    int result = 0;

    result = libmsg_srv_find_registred_client(
            g_ThreadCtx_srv_signaldateNotify.request,
            g_ThreadCtx_srv_signaldateNotify.nfds,
            a_pRequest->filenameClient);

    if ( 0 == result )
    {
        sRequest_t *prequest = &g_ThreadCtx_srv_signaldateNotify.request[g_ThreadCtx_srv_signaldateNotify.nfds];
        memcpy( prequest,a_pRequest,sizeof( sRequest_t));

        g_ThreadCtx_srv_signaldateNotify.nfds++;

        result = pthread_kill(g_ThreadCtx_srv_signaldateNotify.pthreadID,SIGUSR1);
    }
    else
    {
        result = -1;
    }

    return result;
}

//************************************************************
//*
//************************************************************
int libmsg_srvtime_srv_wait()
{
    int result = 0;

    result = pthread_join(g_ThreadCtx_srv_svc_getdate.pthreadID,0);

    result = pthread_join(g_ThreadCtx_srv_svc_Signaldate.pthreadID,0);

    //*****************************
    // LIBMESSAGE_ID_END
    //*****************************

    // wait on end of thread
    return result;
}
//************************************************************
//*
//************************************************************
int libmsg_srvtime_cli_wait()
{
    int result = 0;

    result = pthread_join(g_ThreadCtx_cli_SignaldateNotify.pthreadID,0);


    //*****************************
    // LIBMESSAGE_ID_END
    //*****************************

    // wait on end of thread
    return result;
}
static void timerfunc(__sigval_t a_sigval)
{
    int result = 0;
    sRequest_t *prequest = (sRequest_t*)a_sigval.sival_ptr;
    int fd_client = 0;

    sResponse_t response = {0};
    sSignaldateResponse_t *pSignaldateResponse = (sSignaldateResponse_t *)response.data;

    char        msgbuffer[APISYSLOG_MSG_SIZE]   = {0};


    //***********************************************************
    //                open client
    //***********************************************************
    TRACE_DBG1("_1_ prequest->filenameClient=%s",prequest->filenameClient);

    //memcpy(&request, buffRequest,sizeof(request));

    errno = 0;
    fd_client = mq_open(prequest->filenameClient,O_WRONLY);

    if( -1 == fd_client )
    {
        result = errno;
        snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                "mq_open(%.50s) result=%d errno=%d %s \n",
                prequest->filenameClient,
                result , errno,strerror(errno));
        TRACE_ERR(msgbuffer);
    }

    if( 0 == result )
    {
        TRACE_DBG1("_2_ result=0x%X",result);

        clock_gettime(CLOCK_MONOTONIC_RAW,&pSignaldateResponse->timespesc);

        response.header.datasize =
                sizeof(sHeader_t) + sizeof(sSignaldateResponse_t);

        struct timespec ts_timeout = {0,1e9 / 1000 * 10}; // 10ms

       // send msg to server
        errno = 0;
        result = mq_timedsend(fd_client, (char*)&response,
                response.header.datasize,0,
                &ts_timeout);

        TRACE_DBG2("_3_mq_send()=%d size=%d errno=%u %s\n",
                result,
                response.header.datasize,
                errno,strerror(errno));

        if ( 0 != result)
        {
            snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                    "error mq_send(%s) result=%d errno=%d %s \n",
                    prequest->filenameClient,
                    result , errno,strerror(errno));
            TRACE_ERR(msgbuffer);
        }
    }
    //        fprintf(stderr,"_11_ \n");

    mq_close(fd_client);

    TRACE_DBG4("result=%d client=%s date=%ld.%lld",
//           a_sigval.sival_int,
//           a_sigval.sival_ptr,
            result,
           prequest->filenameClient,
           pSignaldateResponse->timespesc.tv_sec,
           pSignaldateResponse->timespesc.tv_nsec);


}
static void
handler(int sig, siginfo_t *si, void *uc)
{
    /* Note: calling printf() from a signal handler is not
              strictly correct, since printf() is not async-signal-safe;
              see signal(7) */

    TRACE_DBG2("Caught signal %d", sig);

    //signal(sig, SIG_IGN);
}
static void * libmsg_srv_threadFunction_signaldate(void * a_pArg)
{
    int         result      = 0;
    int         timeout     = -1;
    int         signalsend  = 0;
    struct mq_attr  vAttr       = {0};

    sigset_t        sigset      = {0};
    siginfo_t       siginfo     = {0};
    struct sigevent sigevent    = {0};
    struct sigaction sSigaction = {0};


    timer_t         timerid     = 0;
    struct itimerspec its       = {0};

    sRequest_t              *pRequest = 0;
    sSignaldateRequest_t    *pSignaldateRequest = 0;

    sDataThreadCtx_t *pContex = (sDataThreadCtx_t *)a_pArg;


//    sSigaction.sa_flags = SA_SIGINFO;
//    sSigaction.sa_sigaction = handler;
//    sigemptyset(&sSigaction.sa_mask);
//    sigaction(SIGALRM, &sSigaction, NULL) ;

    sigemptyset(&sigset);
    sigaddset(&sigset, SIGUSR1);
    sigaddset(&sigset, SIGALRM);

    result = pthread_sigmask(SIG_BLOCK, &sigset, NULL);

    do{

        vAttr.mq_flags  = O_CLOEXEC;
        vAttr.mq_curmsgs = 1;
        vAttr.mq_maxmsg = 10;
        vAttr.mq_msgsize = HARD_MAX;

        //result = sigwait(&sigset, &signalsend);
        //int sigwaitinfo(const sigset_t *set, siginfo_t *info);

        memset(&siginfo,0,sizeof(siginfo));
        result = sigwaitinfo(&sigset, &siginfo);
        TRACE_DBG2(": _1_ sigwaitinfo() = %d %d %s",result,errno,strerror(errno));

        if(-1 == result)
        {
            // error
        }
        else
        {
            if( SIGUSR1 == result ) // add client
            {
                pRequest = &pContex->request[pContex->nfds-1];
                pSignaldateRequest = (sSignaldateRequest_t*)pRequest->data;
                // create timer
                sigevent.sigev_notify_function = timerfunc;
                sigevent.sigev_notify = SIGEV_THREAD; // SIGEV_SIGNAL;
                sigevent.sigev_signo = SIGALRM;
                sigevent.sigev_value.sival_ptr = &timerid;
                sigevent.sigev_value.sival_ptr = pRequest;

                errno=0;
                result = timer_create(CLOCK_MONOTONIC, &sigevent, &timerid);

TRACE_DBG2(": _2_ timer_create()=%d %d %s pRequest=%#p",
        result,errno,strerror(errno),pRequest);

                its.it_value.tv_sec     = pSignaldateRequest->timespesc.tv_sec;
                its.it_value.tv_nsec    = pSignaldateRequest->timespesc.tv_nsec;
                its.it_interval.tv_sec  = its.it_value.tv_sec;
                its.it_interval.tv_nsec = its.it_value.tv_nsec;

                errno=0;
                result = timer_settime(timerid, 0, &its, NULL);
TRACE_DBG2(": _3_ timer_create()=%d timeout=%ld.%lld %d %s",
        result,
        pSignaldateRequest->timespesc.tv_sec,
        pSignaldateRequest->timespesc.tv_nsec,
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
        }

    }while(1);


    return 0;
}
//************************************************************
//*
//************************************************************
int libmsg_srvtime_register_signal(sDataThreadCtx_t *a_pDataThreadCtx)
{
    int result = 0;
    char msgbuffer[APISYSLOG_MSG_SIZE] = {0};

    //*****************************
    // create new tread for listening incomming messages
    //*****************************
    errno = 0;
    result =  pthread_create(&a_pDataThreadCtx->pthreadID,
            NULL,
            &libmsg_srv_threadFunction_signaldate,
            (void*)a_pDataThreadCtx);

    if( 0 != result )
    {
        snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                ": pthread_create() error =%d %s",
                result,strerror(result));

        TRACE_ERR(msgbuffer);
    }

    return result;
}
