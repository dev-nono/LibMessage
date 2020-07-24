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
#include <time.h>
#include <string.h>

#include "apisyslog.h"
#include "utils.h"

#include <errno.h>
//#include "libmessage_int.h"
#include "libmessage_svc_time.h"



 static int libmessage_cbfcnt_signaldate(void *a_pContext)
 {
     int result = 0;

     sRequest_t *  pRequest   =     &( (sDataThreadCtx_t*) a_pContext)->dataService.request;
     sResponse_t * pResponse  =     &( (sDataThreadCtx_t*) a_pContext)->dataService.response;


     // add client to list of observer
   result = libmessage_server_register_fifosignal(a_pContext);


     return  result;
 }

 static int libmessage_cbfcnt_setdate(//const    void * a_pRequest,void * a_pResponse)
         void *a_pContext)
 {
     int result = ENODATA;
     char msgbuffer[APISYSLOG_MSG_SIZE] = {0};
//     sRequest_t *  pRequest   = (sRequest_t*) a_pRequest;
//     sResponse_t * pResponse  = (sResponse_t*)a_pResponse;
     sRequest_t *  pRequest   =     &( (sDataThreadCtx_t*) a_pContext)->dataService.request;
     sResponse_t * pResponse  =     &( (sDataThreadCtx_t*) a_pContext)->dataService.response;

     sSetdateRequest_t *pData = (sSetdateRequest_t*)&pRequest->data;

     pResponse->header.result = result;

     pResponse->header.datasize = sizeof(sHeader_t);


     snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
             " : setdate=%ld.%09ld result=%d\n",
             pData->timespesc.tv_sec,
             pData->timespesc.tv_nsec,
             result);


     TRACE_LOG(msgbuffer);

     return  result;
 }

static int libmessage_cbfcnt_getdate( void *a_pContext)
{
    int result = 0;
    char msgbuffer[APISYSLOG_MSG_SIZE] = {0};

    //sRequest_t *  pRequest   = (sRequest_t*) a_pRequest;

    // no inputdata in request
    sRequest_t *  pRequest   =     &( (sDataThreadCtx_t*) a_pContext)->dataService.request;
    sResponse_t * pResponse  =     &( (sDataThreadCtx_t*) a_pContext)->dataService.response;

    sGetdateResponse_t  *pData      = (sGetdateResponse_t *)&pResponse->data;

    pResponse->header.datasize =
            sizeof(sHeader_t) + sizeof(sGetdateResponse_t);

    result = clock_gettime(CLOCK_MONOTONIC_RAW,
            &pData->timespesc);

    snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
            " : date=%ld.%09ld result=%d\n",
            pData->timespesc.tv_sec,
            pData->timespesc.tv_nsec,
            result);

    pResponse->header.result = result;

    TRACE_LOG(msgbuffer);

    return result;
}

//*********************************************************
//*
//*********************************************************
int main(int argc, char *argv[])
{
    int result = 0;

    (void)argv;
    (void)argc;

    apisyslog_init("");

    //    result = srv_getdate();
    //
    //
    //

    //    result = libmessage_srvtime_initialize() // for signal  TODO
    result = libmessage_srvtime_register_getdate(&libmessage_cbfcnt_getdate);
    result = libmessage_srvtime_register_setdate(&libmessage_cbfcnt_setdate);
    result = libmessage_srvtime_register_signaldate(&libmessage_cbfcnt_signaldate); //TODO


    result = libmessage_srvtime_wait();


    return result;
}

