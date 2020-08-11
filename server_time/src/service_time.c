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

#include "apisyslog.h"
#include "utilstools_listtailqueue.h"
#include "libmsg_srvtime.h"


static sDataThreadCtx_t g_Context_signaldate = {0} ;

ListQ_t g_List_signaldate = {0};

static int servicetime_cbfcnt_svc_getdate(
        const sRequest_t  *a_pRequest,
        sResponse_t *a_pResponse)

{
    int result = 0;
    char msgbuffer[APISYSLOG_MSG_SIZE] = {0};

    (void)a_pRequest; // no input data to use

    sGetdateResponse_t  *pData      = (sGetdateResponse_t *)&a_pResponse->data;

    a_pResponse->header.datasize =
            sizeof(sHeader_t) + sizeof(pData->timespesc);

    result = clock_gettime(CLOCK_MONOTONIC_RAW,
            &pData->timespesc);

    snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
            " : date=%ld.%09ld result=%d\n",
            pData->timespesc.tv_sec,
            pData->timespesc.tv_nsec,
            result);

    a_pResponse->header.result = result;

    TRACE_LOG(msgbuffer);

    return result;
}

static int servicetime_cbfcnt_svc_signaldate(
        const sRequestSignal_t  *a_pRequestSignal,
        sResponse_t *a_pResponse)

{
    int     result = 0;
//    char service[NI_MAXSERV]= {0};
    char host[NI_MAXHOST]   = {0};
    char    service[NAME_MAX]; // NI_MAXSERV
    int nbitem = 0;

    a_pResponse->header.result = 0;
    a_pResponse->header.datasize = sizeof(a_pResponse->header);

    result = getnameinfo((struct sockaddr*)&a_pRequestSignal->peer_addr,
            a_pRequestSignal->peer_addr_len,
            host,sizeof(host),
            service,   sizeof(service),
            NI_NUMERICHOST | NI_NUMERICSERV);

    if( 0 != result )
        result = -1;

    ListQ_item_t *pItem = tq_insertTail(&g_List_signaldate);
    pItem->pData = (void*)a_pRequestSignal;

    nbitem = tq_size(&g_List_signaldate);
    // register client    a_pRequest->filenameClient
    // add new client in list

    // register_srv_signal(servicetime_cbfcnt_srv_signal)

    TRACE_DBG1(": service= %s host= %s nbitem=%d result=%d",service,host,nbitem,result);



    return result;

}

static void * servicetime_threadFunction_signaldate(void * a_pArg)
{

    //configure timer

    do
    {
        //wait signal
        //for(ii=0 ; ii< nbitellist )
        // sentto


        //configure timer

    }while(1);

    return (void*)0;
}

//static int libmessage_cbfcnt_setdate(void* a_pData)
//{
//    int result = 0;
//
//    return result;
//}
//static int libmsg_srv_cbfcnt_signaldate(
//        const sRequest_t  *a_pRequest,
//        sResponse_t *a_pResponse)
//
//{
//    int result = 0;
//    //char msgbuffer[APISYSLOG_MSG_SIZE] = {0};
//
//    sSignaldateRequest_t *pSignalRequest  = (sSignaldateRequest_t*)&a_pRequest->data;
//
//    a_pResponse->header.datasize = sizeof(sHeader_t);
//    a_pResponse->header.result = result;
//
//    result = libmsg_srvtime_srv_signaldate_addClientNotify(a_pRequest);
//
//    if( 0 != result)
//    {
//        TRACE_ERR(": error register signal (%s) timeout=%ld.%09ld result=%d",
//                a_pRequest->filenameClient,
//                pSignalRequest-> timespesc.tv_sec,
//                pSignalRequest->timespesc.tv_nsec,
//                result);
//
//    }
//    else
//    {
//        TRACE_DBG4(": register signal (%s) timeout=%ld.%09ld result=%d",
//                a_pRequest->filenameClient,
//                pSignalRequest-> timespesc.tv_sec,
//                pSignalRequest->timespesc.tv_nsec,
//                result);
//    }
//
//    return result;
//}
//*********************************************************
//*
//*********************************************************
int servicetime_signaldate()
{
    int result = 0;


    // create signaldate thread job


   // result = libmsg_srvtime_srv_register_svc_srvtime( servicetime_cbfcnt_svc_signaldate);

    return result;
}

//*********************************************************
//*
//*********************************************************
int main(void)
{
    int result = 0;

    apisyslog_init("");
    sleep(1);

    tq_init(&g_List_signaldate,sizeof(sRequestSignal_t));

//    result = libmsg_srvtime_srv_register_svc_getdate(servicetime_cbfcnt_svc_getdate);
    result = libmsg_srvtime_srv_register_svc_signal(servicetime_cbfcnt_svc_signaldate);

    //    result = libmessage_register_service_time( SERVER_TIME_SETDATE, libmessage_cbfcnt_setdate);

    //result = servicetime_signaldate();

    //    sleep(1);
    //    result = libmsg_srvtime_srv_register_svc_signal(libmsg_srv_cbfcnt_signaldate);
    sleep(1);
    result = libmsg_srvtime_srv_wait();

    return result;
}

