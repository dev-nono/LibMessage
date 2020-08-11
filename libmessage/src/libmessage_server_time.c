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
#include "utilstools_time.h"

#include "libmsg_srvtime.h"


// client  : /process.id.svc
// server  : /process.svc
//

static sDataThreadCtx_t g_ThreadCtx_srv_svc_getdate          = {0};

static sDataThreadCtxSignal_t g_ThreadCtx_srv_svc_signaldate       = {0};

//static sDataThreadCtx_t g_ThreadCtx_cli_SignaldateNotify     = {0};
//static sDataThreadCtx_t g_ThreadCtx_srv_signaldateNotify     = {0};

//static void * libmsg_cli_threadFunction_signaldate(void * a_pArg);



//************************************************************
//  client side
//      _OUT_ double *a_pDate : buffer data output
//      return:
//          SUCCESS    0
//          EINVAL          22  Invalid argument
//************************************************************
int libmsg_srvtime_cli_getdate(_OUT_ double *a_Date)
{
    int     result                  = 0;
    char msgbuffer[APISYSLOG_MSG_SIZE] = {0};
    char ClientFilename[NAME_MAX];

    sDataService_t      dataService;
    sGetdateResponse_t  *pResponse_getdate  = 0;

    memset(&dataService,0,sizeof(dataService));

    getUniqname("/tmp",SVC_GETDATE ,ClientFilename);

    strncpy(dataService.request.filenameClient,ClientFilename,NAME_MAX-1);
    strncpy(dataService.filenameServer,SERVER_TIME_GETDATE,NAME_MAX-1);

    if( 0 == result )
    {
        result = libmsg_cli_getdata(&dataService);
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
int libmsg_srvtime_srv_register_svc_getdate_recvfrom(libmsg_pFunctCB_t a_pFunctCB)
{
    int result = 0;

    //*****************************
    // prepare data thread
    //*****************************

    g_ThreadCtx_srv_svc_getdate.dataService.pFunctCB = a_pFunctCB;

    strncpy(g_ThreadCtx_srv_svc_getdate.dataService.filenameServer,
            SERVER_TIME_GETDATE,
            sizeof(g_ThreadCtx_srv_svc_getdate.dataService.filenameServer)-1);

    result = libmsg_srv_register_svc_recvfrom(&g_ThreadCtx_srv_svc_getdate);

    return result;
}


//************************************************************
//*
//************************************************************

int libmsg_srvtime_cli_signaldate(  const double        a_Date,
        libmsg_pFunctCB_response_t a_pFunctCB)
{
    int     result                  = 0;
    char msgbuffer[APISYSLOG_MSG_SIZE] = {0};
    char ClientFilename[NAME_MAX];

    sDataService_t      dataService;
    sGetdateResponse_t  *pResponse_getdate  = 0;

    memset(&dataService,0,sizeof(dataService));

    getUniqname("/tmp",SVC_SIGNALDATE ,ClientFilename);

    strncpy(dataService.request.filenameClient,ClientFilename,NAME_MAX-1);
    strncpy(dataService.filenameServer,SERVER_TIME_SIGNALDATE,NAME_MAX-1);

    if( 0 == result )
    {
        result = libmsg_cli_getdata(&dataService);
    }

    if( 0 == result )
    {
        // create thread
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

    result = pthread_join(g_ThreadCtx_srv_svc_signaldate.pthreadID,0);

    //*****************************
    // LIBMESSAGE_ID_END
    //*****************************

    // wait on end of thread
    return result;
}

//static void * libmsg_cli_threadFunction_signaldate(void * a_pArg)
//{
//    int result = 0;
//
//    return (void*)0;
//}

//************************************************************
//*
//************************************************************
int libmsg_srvtime_srv_register_svc_signal(libmsg_pFunctSignalCB_t a_pFunctCB)
{
    int result = 0;

    //*****************************
    // incomming message
    //*****************************

    g_ThreadCtx_srv_svc_signaldate.dataService.pFunctCB = a_pFunctCB;

    strncpy(g_ThreadCtx_srv_svc_signaldate.dataService.filenameServer,
            SERVER_TIME_SIGNALDATE,
            sizeof(g_ThreadCtx_srv_svc_signaldate.dataService.filenameServer)-1);

    result = libmsg_srv_register_svc_recvfrom_Signal(&g_ThreadCtx_srv_svc_signaldate);

//    //*****************************
//    // sending notification
//    //*****************************
//    result = libmsg_srvtime_register_signal(&g_ThreadCtx_srv_signaldateNotify);
//

    return result;
}
//************************************************************
//  client side
//      _OUT_ double *a_pDate : buffer data output
//      return:
//          SUCCESS    0
//          EINVAL          22  Invalid argument
//************************************************************
//int libmsg_srvtime_cli_signaldate( const char* a_UniqID,
//                                const double a_Date,
//                                libmsg_pFunctCB_t a_pFunctCB)
//{
//    int     result                  = 0;
//    char msgbuffer[APISYSLOG_MSG_SIZE] = {0};
//
//    sDataService_t      dataServiceRequest = {0};
//
//    struct sSignaldateRequest *pRequestData = 0;
//
//    result = getMqClientname(a_UniqID,SVC_SIGNALDATE,dataServiceRequest.request.filenameClient);
//
//    if( 0 == result )
//    {
//        pRequestData = (struct sSignaldateRequest*)dataServiceRequest.request.data;
//
//        time_cnv_double_to_ts(a_Date,&pRequestData->timespesc);
//
////        result = libmsg_cli_getdata( SERVER_TIME_SIGNALDATE,&dataServiceRequest);
//    }
//
//    if( 0 == result )
//    {
//        snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
//                " : result = %d ",dataServiceRequest.response.header.result);
//        TRACE_LOG(msgbuffer);
//    }
//
//    //****************************************************************
//    g_ThreadCtx_cli_SignaldateNotify.dataService.pFunctCB = a_pFunctCB;
//
//
////    strncpy(g_ThreadCtx_cli_SignaldateNotify.dataService.filenameServer,
////            dataServiceRequest.request.filenameClient,
////            sizeof(g_ThreadCtx_cli_SignaldateNotify.dataService.filenameServer)-1);
//
//
//    strncpy(g_ThreadCtx_cli_SignaldateNotify.dataService.request.filenameClient,
//            dataServiceRequest.request.filenameClient,
//            sizeof(g_ThreadCtx_cli_SignaldateNotify.dataService.request.filenameClient)-1);
//
//    result = libmsg_cli_register_svc(&g_ThreadCtx_cli_SignaldateNotify);
//
//
//    return result;
//}


//************************************************************
//*
//************************************************************
int libmsg_srvtime_cli_wait()
{
    int result = 0;

    struct timespec date = {0};

    getDateDouble(date);

    //*****************************
    // LIBMESSAGE_ID_END
    //*****************************

    // wait on end of thread
    return result;
}
