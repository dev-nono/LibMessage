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


#include "libmsg_srvtime.h"


static int libmsg_srv_cbfcnt_getdate(
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
int main(void)
{
    int result = 0;

    apisyslog_init("");
    sleep(1);

    result = libmsg_srvtime_srv_register_svc_getdate(libmsg_srv_cbfcnt_getdate);
    //    result = libmessage_register_service_time( SERVER_TIME_SETDATE, libmessage_cbfcnt_setdate);
    //    result = libmessage_register_service_time( SERVER_TIME_SIGNAL,  libmessage_cbfcnt_signal,     libmessage_cbfcnt_signal);

//    sleep(1);
//    result = libmsg_srvtime_srv_register_svc_signal(libmsg_srv_cbfcnt_signaldate);
    sleep(1);
    result = libmsg_srvtime_srv_wait();

    return result;
}

