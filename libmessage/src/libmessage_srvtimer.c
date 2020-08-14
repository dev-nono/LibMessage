/*
 * server_time.c
 *
 *  Created on: 4 juin 2020
 *      Author: bruno
 */
#define _GNU_SOURCE

#include <stdio.h>

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
#include <errno.h>

#include "apisyslog.h"
#include "utilstools_time.h"

#include "libmsg_srvtimer.h"


// client  : /process.id.svc
// server  : /process.svc
//

static sThreadDataCtx_t         g_ThreadCtx_srv_svc_getdate          = {0};
static sThreadDataCtxSignal_t   g_ThreadCtx_srv_svc_timer       = {0};
static sThreadDataCtxSignal_t   g_ThreadCtx_cli_svc_timer       = {0};

/**
 * \fn      int libmsg_srvtimer_cli_getdate(const char* a_Servicename , double *a_Date)
 *
 * \brief   call service "getdate"
 *
 * \param   const char  *a_servername : name of server to call svc getdate
 * \param   double      *a_Date       : output pointer to double
 *
 * \return      0 is ok
 *              or error code
 */
int libmsg_srvtimer_cli_getdate(const char* a_servername , double *a_Date)
{
    int     result                  = 0;
    char msgbuffer[APISYSLOG_MSG_SIZE] = {0};
    char ClientFilename[NAME_MAX];

    sDataService_t      dataService;
    sGetdateResponse_t  *pResponse_getdate  = 0;

    memset(&dataService,0,sizeof(dataService));

    getUniqname("/tmp",SVC_GETDATE ,ClientFilename);

    strncpy(dataService.request.filenameClient,ClientFilename,NAME_MAX-1);
    strncpy(dataService.filenameServer,a_servername,NAME_MAX-1);

    if( 0 == result )
    {
        result = libmsg_cli_getdata(&dataService,1);
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

/**
 * \fn       libmsg_srvtimer_srv_register_svc_getdate(char *a_filenameserver,libmsg_pFunctCB_t a_pFunctCB);
 *
 * \brief       server side, register svc getdate
 *
 * \param   char                *a_filenameserver   :  name of socker svc server
 * \param   libmsg_pFunctCB_t   a_pFunctCB          :  callback to call for this svc
 *
 * \return  0 == OK
 *          error code
 */
int libmsg_srvtimer_srv_register_svc_getdate(char *a_filenameserver,  libmsg_pFunctCB_t a_pFunctCB)
{
    int result = 0;

    //*****************************
    // prepare data thread
    //*****************************

    g_ThreadCtx_srv_svc_getdate.dataService.pFunctCB = a_pFunctCB;

    strncpy(g_ThreadCtx_srv_svc_getdate.dataService.filenameServer,
            a_filenameserver,
            sizeof(g_ThreadCtx_srv_svc_getdate.dataService.filenameServer)-1);

    result = libmsg_srv_register_svc(&g_ThreadCtx_srv_svc_getdate);

    return result;
}

/**
 * \fn       libmsg_srvtimer_cli_timer(
 *          char                        *a_filenameserver,
 *          const double                a_Date,
 *          libmsg_pFunctCB_response_t  a_pFunctCB
 *          sSignal_t                   *a_pDataSvcTimer )
 *
 * \brief
 *
 * \param
 * \return
 */
int libmsg_srvtimer_cli_timer(
        char                        *a_filenameserver,
        const struct timespec       a_Timeout,
        libmsg_pFunctCB_response_t  a_pFunctCBresponse,
        sSignal_t                   *a_pDataSvcTimer )
{
    int     result                  = 0;
//    char msgbuffer[APISYSLOG_MSG_SIZE] = {0};
    sDataService_t      dataService = {0};
    sTimerRequest_t     *pRequestData = 0;


    getUniqname("/tmp",SVC_TIMER ,dataService.request.filenameClient);

    strncpy(dataService.filenameServer,a_filenameserver,NAME_MAX-1);

    pRequestData = (sTimerRequest_t*)dataService.request.data;

    pRequestData->timespesc = a_Timeout;

    dataService.request.header.datasize = sizeof(dataService.request.header)
                + sizeof(sTimerRequest_t);

    if( 0 == result )
    {
        result = libmsg_cli_getdata(&dataService,0);
    }

    if( 0 == result )
    {
        memcpy(&g_ThreadCtx_cli_svc_timer.dataService.dataSignal,
                &dataService.dataSignal,sizeof(sSignal_t));

        memcpy(a_pDataSvcTimer,&dataService.dataSignal,sizeof(sSignal_t));

        g_ThreadCtx_cli_svc_timer.dataService.pFunctCBresponse = a_pFunctCBresponse;

        strncpy(g_ThreadCtx_cli_svc_timer.dataService.filenameServer,
                dataService.request.filenameClient,
                sizeof(g_ThreadCtx_cli_svc_timer.dataService.filenameServer));

        result = libmsg_cli_register_svc_Signal(&g_ThreadCtx_cli_svc_timer);
    }

    memcpy(a_pDataSvcTimer,&dataService.dataSignal,sizeof(sSignal_t));

    return result;
}

//************************************************************
//*
//************************************************************
int libmsg_srvtimer_srv_wait()
{
    int result = 0;

    result = pthread_join(g_ThreadCtx_srv_svc_getdate.pthreadID,0);

    result = pthread_join(g_ThreadCtx_srv_svc_timer.pthreadID,0);

    //*****************************
    // LIBMESSAGE_ID_END
    //*****************************

    // wait on end of thread
    return result;
}

/**
 * \fn          int libmsg_srvtimer_srv_register_svc_timer(libmsg_pFunctSignalCB_t a_pFunctCB)
 * \brief
 *
 * \param
 * \return
 */
int libmsg_srvtimer_srv_register_svc_timer(char *a_filenameserver,libmsg_pFunctSignalCB_t a_pFunctCB)
{
    int result = 0;

    //*****************************
    // incomming message
    //*****************************

    g_ThreadCtx_srv_svc_timer.dataService.pFunctCB = a_pFunctCB;

    strncpy(g_ThreadCtx_srv_svc_timer.dataService.filenameServer,
            a_filenameserver,
            sizeof(g_ThreadCtx_srv_svc_timer.dataService.filenameServer)-1);

    result = libmsg_srv_register_svc_Signal(&g_ThreadCtx_srv_svc_timer);

//    //*****************************
//    // sending notification
//    //*****************************
//    result = libmsg_srvtime_register_signal(&g_ThreadCtx_srv_signaldateNotify);
//

    return result;
}

//************************************************************
//*
//************************************************************
int libmsg_srvtimer_cli_wait()
{
    int result = 0;

    // wait on end of thread
    return result;
}
