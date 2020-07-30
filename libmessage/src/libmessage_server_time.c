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


// #define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <limits.h>

#include "apisyslog.h"
#include "utils.h"


#include "libmsg_srvtime.h"
//#include "libmessage_int.h"
//#include "libmessage.h"


// client  : /process.id.svc
// server  : /process.svc
//

//static sDataThreadCtx_t g_TheadCtx_setdate= {0};

static sDataThreadCtx_t g_TheadCtx_srv_getdate          = {0};
static sDataThreadCtx_t g_TheadCtx_srv_Signaldate       = {0};
static sDataThreadCtx_t g_TheadCtx_srv_SignaldateNotify = {0};

static sDataThreadCtx_t g_TheadCtx_cli_SignaldateNotify = {0};


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
int libmsg_srvtime_srv_register_getdate(libmsg_pFunctCB_t a_pFunctCB)
{
    int result = 0;

    //*****************************
    // prepare data thread
    //*****************************

    g_TheadCtx_srv_getdate.dataService.pFunctCB = a_pFunctCB;

    strncpy(g_TheadCtx_srv_getdate.dataService.filenameServer,
            SERVER_TIME_GETDATE,
            sizeof(g_TheadCtx_srv_getdate.dataService.filenameServer)-1);

    result = libmsg_srv_register_svc(&g_TheadCtx_srv_getdate);

    return result;
}
//************************************************************
//*
//************************************************************
int libmsg_srvtime_srv_register_signal(libmsg_pFunctCB_t a_pFunctCB)
{
    int result = 0;

    //*****************************
    // incomming message
    //*****************************

    g_TheadCtx_srv_Signaldate.dataService.pFunctCB = a_pFunctCB;

    strncpy(g_TheadCtx_srv_Signaldate.dataService.filenameServer,
            SERVER_TIME_SIGNALDATE,
            sizeof(g_TheadCtx_srv_Signaldate.dataService.filenameServer)-1);


    result = libmsg_srv_register_svc(&g_TheadCtx_srv_Signaldate);

    //*****************************
    // sending notification
    //*****************************
//    result = libmsg_srv_register_signal(&g_TheadCtx_SignaldateNotify);


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
    g_TheadCtx_cli_SignaldateNotify.dataService.pFunctCB = a_pFunctCB;


    strncpy(g_TheadCtx_cli_SignaldateNotify.dataService.filenameServer,
            dataServiceRequest.request.filenameClient,
            sizeof(g_TheadCtx_cli_SignaldateNotify.dataService.filenameServer)-1);


    result = libmsg_srv_register_svc(&g_TheadCtx_cli_SignaldateNotify);


    return result;
}
//************************************************************
//*
//************************************************************
int libmsg_srvtime_srv_wait()
{
    int result = 0;

    result = pthread_join(g_TheadCtx_srv_getdate.pthreadID,0);

    result = pthread_join(g_TheadCtx_srv_Signaldate.pthreadID,0);

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

    result = pthread_join(g_TheadCtx_srv_SignaldateNotify.pthreadID,0);


    //*****************************
    // LIBMESSAGE_ID_END
    //*****************************

    // wait on end of thread
    return result;
}
