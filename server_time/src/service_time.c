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

#include "libmessage_int.h"
#include "libmessage_svc_time.h"



 static int libmessage_cbfcnt_signaldate(const    void * a_pRequest,
                                                 void * a_pResponse)
 {
     int result = 0;

     sRequest_t *  pRequest   = (sRequest_t*) a_pRequest;
     sResponse_t * pResponse  = (sResponse_t*)a_pResponse;

     (void)pRequest;
     (void)pResponse;

     return  result;
 }

 static int libmessage_cbfcnt_setdate(const    void * a_pRequest,
                                                 void * a_pResponse)
 {
     int result = 0;
     sRequest_t *  pRequest   = (sRequest_t*) a_pRequest;
     sResponse_t * pResponse  = (sResponse_t*)a_pResponse;

     (void)pRequest;
     (void)pResponse;

//
//     struct timespec *pTp = (struct timespec *)a_pData;
//
//
//     printf("%ld.%09ld",pTp->tv_sec,pTp->tv_nsec);

     return  result;
 }

static int libmessage_cbfcnt_getdate( const    void * a_pRequest,
                                               void * a_pResponse)
{
    int result = 0;
    char msgbuffer[APISYSLOG_MSG_SIZE] = {0};

    sRequest_t *  pRequest   = (sRequest_t*) a_pRequest;
    sResponse_t * pResponse  = (sResponse_t*)a_pResponse;

    (void)pRequest;


    result = clock_gettime(CLOCK_MONOTONIC_RAW,
            &pResponse->uResponse.getdate.timespesc);

    snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
            " : date=%ld.%09ld len=%d\n",
            pResponse->uResponse.getdate.timespesc.tv_sec,
            pResponse->uResponse.getdate.timespesc.tv_nsec,
            result);

    pResponse->result = result;

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
    result = libmessage_srvtime_register_getdate(&libmessage_cbfcnt_getdate);
    result = libmessage_srvtime_register_setdate(&libmessage_cbfcnt_setdate);
    result = libmessage_srvtime_register_signaldate(&libmessage_cbfcnt_signaldate);


    result = libmessage_srvtime_wait();


    return result;
}

