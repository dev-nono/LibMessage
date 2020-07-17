/*
 * server_time.c
 *
 *  Created on: 4 juin 2020
 *      Author: bruno
 */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>           /* Pour les constantes O_* */
#include <sys/stat.h>        /* Pour les constantes des modes */

#include <errno.h>
#include <string.h>
#include <pthread.h>

// #define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <limits.h>

#include "apisyslog.h"

#include "utils.h"


#include "libmessage_int.h"
#include  "libmessage_svc_time.h"


static sDataService_t  g_DataService_getdata = {0};


static sDataThreadCtx_t g_arrayTheadCtx[eLIBMSG_ID_END]= {0};
static sDataThreadCtx_t *getTheadCtx(eLIBMSG_ID_t a_ID)
{
    return &g_arrayTheadCtx[a_ID];
}

static sDataService_t g_DataService_signaldata = {0};

static char  g_arrayNameService[eLIBMSG_ID_END][3][NAME_MAX]=
{
        {SVC_TIME_GETDATE_PREFIX,      SVR_TIME_GETDATE_SEM,    SRV_TIME_GETDATE_FILENAME},
        {SVC_TIME_SETDATE_PREFIX,      SVR_TIME_SETDATE_SEM,    SRV_TIME_SETDATE_FILENAME},
        {SVC_TIME_SIGNALDATE_PREFIX,   SVR_TIME_SIGNALDATE_SEM, SRV_TIME_SIGNALDATE_FILENAME}

};
static const char *getNameService(eLIBMSG_ID_t a_ID,eLIBMSG_COL_t a_ColID)
{
    return g_arrayNameService[a_ID][a_ColID];
}

//************************************************************
//*
//************************************************************
int libmessage_srvtime_wait()
{
    TRACE_IN("_IN")
                                    int result = 0;

    // joint thread 1
    pthread_join(getTheadCtx(eLIBMSG_ID_GETDATA)->pthreadID,0);
    pthread_join(getTheadCtx(eLIBMSG_ID_SETDATA)->pthreadID,0);
    pthread_join(getTheadCtx(eLIBMSG_ID_SIGNAL)->pthreadID,0);


    TRACE_OUT("_OUT result=%d",result)

    return result;
}


//************************************************************
//*
//************************************************************
int libmessage_srvtime_register_getdate(libmessage_pFunctCB_t a_pFunctCB)
{
    int result = 0;

    //*****************************
    // prepare data thread
    //*****************************

   sDataThreadCtx_t    *pDataThreadCtx = getTheadCtx(eLIBMSG_ID_GETDATA);

   pDataThreadCtx->dataService.pFunctCB = a_pFunctCB;

   strncpy(pDataThreadCtx->dataService.filenameServer,
           getNameService(eLIBMSG_ID_GETDATA,eLIBMSG_COL_SRV_FILENAME),
           sizeof(pDataThreadCtx->dataService.filenameServer)-1);


   strncpy(pDataThreadCtx->dataService.filenameSemaphore,
           getNameService(eLIBMSG_ID_GETDATA,eLIBMSG_COL_SEM),NAME_MAX-1);

    result = libmessage_server_register_svc(pDataThreadCtx);

    return result;
}
//************************************************************
//*
//************************************************************
int libmessage_srvtime_register_setdate(libmessage_pFunctCB_t a_pFunctCB)
{
    int result = 0;

    sDataThreadCtx_t    *pDataThreadCtx = getTheadCtx(eLIBMSG_ID_SETDATA);

    pDataThreadCtx->dataService.pFunctCB = a_pFunctCB;

    strncpy(pDataThreadCtx->dataService.filenameServer,
            getNameService(eLIBMSG_ID_SETDATA,eLIBMSG_COL_SRV_FILENAME),
            sizeof(pDataThreadCtx->dataService.filenameServer)-1);


    strncpy(pDataThreadCtx->dataService.filenameSemaphore,
            getNameService(eLIBMSG_ID_SETDATA,eLIBMSG_COL_SEM),NAME_MAX-1);

    result = libmessage_server_register_svc(pDataThreadCtx);

    return result;
}
//************************************************************
//*
//************************************************************
int libmessage_srvtime_register_signaldate(libmessage_pFunctCB_t a_pFunctCB)
{
    int result = 0;

    sDataThreadCtx_t    *pDataThreadCtx = getTheadCtx(eLIBMSG_ID_SIGNAL);

    pDataThreadCtx->dataService.pFunctCB = a_pFunctCB;

    strncpy(pDataThreadCtx->dataService.filenameServer,
            getNameService(eLIBMSG_ID_SIGNAL,eLIBMSG_COL_SRV_FILENAME),
            sizeof(pDataThreadCtx->dataService.filenameServer)-1);


    strncpy(pDataThreadCtx->dataService.filenameSemaphore,
            getNameService(eLIBMSG_ID_SIGNAL,eLIBMSG_COL_SEM),NAME_MAX-1);

    result = libmessage_server_register_svc(pDataThreadCtx);

    return result;
}


//************************************************************
//  client side
//      _OUT_ double *a_pDate : buffer data output
//      return:
//          SUCCESS    0
//          EINVAL          22  Invalid argument
//************************************************************
int libmessage_getdate(_OUT_ double     *a_pDate)
{
    int         result                          = SUCCESS;
    char        msgbuffer[APISYSLOG_MSG_SIZE]   = {0};


    if( (!a_pDate) )
    {
        result = EINVAL ;

        snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                " : Invalid argument error =%d %s",

                result,strerror(result));

        fprintf(stderr,"%s : %s \n",__FUNCTION__, msgbuffer);
        TRACE_ERR(msgbuffer);
    }

    if( SUCCESS == result )
    {
        getFifoname(g_DataService_getdata.request.filenameClient);

        strcpy(g_DataService_getdata.filenameServer,getNameService(eLIBMSG_ID_GETDATA,eLIBMSG_COL_SRV_FILENAME));

        errno = 0;
        g_DataService_getdata.pSemsvc = sem_open(getNameService(eLIBMSG_ID_GETDATA,eLIBMSG_COL_SEM),0);

        if( SEM_FAILED == g_DataService_getdata.pSemsvc)
        {
            snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                    ": sem_open(%s) result=0x%p errno=%d %s",
                    getNameService(eLIBMSG_ID_GETDATA,eLIBMSG_COL_SRV_FILENAME),
                    (void*)g_DataService_getdata.pSemsvc,
                    errno,strerror(errno));
            fprintf(stderr,"%s : %s \n",__FUNCTION__, msgbuffer);
            TRACE_ERR(msgbuffer);

            result = errno;
        }

        g_DataService_getdata.pFunctCB = 0;
    }

    if( SUCCESS == result )
    {
        g_DataService_getdata.request.header.datasize =
                    sizeof(g_DataService_getdata.request);

        result = libmessage_svc_client_getdata(&g_DataService_getdata);
    }

    if( SUCCESS == result )
    {
        sGetdateResponse_t *pResponse= (sGetdateResponse_t*)g_DataService_getdata.response.data;


        *a_pDate = (    (double)pResponse->timespesc.tv_sec)
                    + ((double)pResponse->timespesc.tv_nsec*1e-9);

        TRACE_DBG1(" : '%ld.%09ld",
                pResponse->timespesc.tv_sec,
                pResponse->timespesc.tv_nsec);
    }

    return result !=0;
}


//************************************************************
//  client side
//      _OUT_ double *a_pDate : buffer data output
//      return:
//          SUCCESS    0
//          EINVAL          22  Invalid argument
//************************************************************
int libmessage_setdate( _IN_ double  a_Date)
{
    int result = SUCCESS;

    char msgbuffer[APISYSLOG_MSG_SIZE] = {0};

    sDataService_t vDataService = {0};

    if( SUCCESS == result )
    {
        getFifoname(vDataService.request.filenameClient);

        strcpy(vDataService.filenameServer,getNameService(eLIBMSG_ID_SETDATA,eLIBMSG_COL_SRV_FILENAME));

        errno = 0;
        vDataService.pSemsvc = sem_open(getNameService(eLIBMSG_ID_SETDATA,eLIBMSG_COL_SEM),0);

        if( SEM_FAILED == vDataService.pSemsvc)
        {
            snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                    ": sem_open(%s) result=0x%p errno=%d %s",
                    getNameService(eLIBMSG_ID_SETDATA,eLIBMSG_COL_SRV_FILENAME),
                    (void*)vDataService.pSemsvc,
                    errno,strerror(errno));
            fprintf(stderr,"%s : %s \n",__FUNCTION__, msgbuffer);
            TRACE_ERR(msgbuffer);

            result = errno;
        }

        vDataService.pFunctCB = 0;
    }
        /* TODO

    if( SUCCESS == result )
    {
        vDataService.request.uRequest.setdate.timespesc.tv_sec  = (__time_t)a_Date;
        vDataService.request.uRequest.setdate.timespesc.tv_nsec =
                (a_Date - vDataService.request.uRequest.setdate.timespesc.tv_sec)
                *1e9;
        result = libmessage_svc_client_getdata(&vDataService);
    }

    if( SUCCESS == result )
    {
        if(SUCCESS != vDataService.response.result)
        {
            TRACE_ERR(" service error = %d %s",
                    vDataService.response.result,
                    strerror(vDataService.response.result));

        }
        result = vDataService.response.result;
    }
    else
    {
    }
TODO */

    return result ;
}

//************************************************************
//  client side
//      return:
//          SUCCESS    0
//          EINVAL          22  Invalid argument
//************************************************************
int libmessage_signaldate(
        _IN_    double     a_Date,         // buffer data output
        _IN_    libmessage_pFunctSignalCB_t a_pFunct)// callback
{
    int result = SUCCESS;
//
//    char msgbuffer[APISYSLOG_MSG_SIZE] = {0};
//
//    if( SUCCESS == result )
//    {
//        getFifoname(g_DataService_signaldata.request.filenameClient);
//
//        strcpy(g_DataService_signaldata.filenameServer,getNameService(eLIBMSG_ID_SIGNAL,eLIBMSG_COL_SRV_FILENAME));
//
//        g_DataService_signaldata.request.uRequest.signaldate.timespesc.tv_sec  = (__time_t)a_Date;
//        g_DataService_signaldata.request.uRequest.signaldate.timespesc.tv_nsec =
//                (a_Date - (__time_t)a_Date) * 1e9;
//
//        errno = 0;
//        g_DataService_signaldata.pSemsvc = sem_open(getNameService(eLIBMSG_ID_SIGNAL,eLIBMSG_COL_SEM),0);
//
//        if( SEM_FAILED == g_DataService_signaldata.pSemsvc)
//        {
//            snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
//                    ": sem_open(%s) result=0x%p errno=%d %s",
//                    getNameService(eLIBMSG_ID_SIGNAL,eLIBMSG_COL_SRV_FILENAME),
//                    (void*)g_DataService_signaldata.pSemsvc,
//                    errno,strerror(errno));
//            fprintf(stderr,"%s : %s \n",__FUNCTION__, msgbuffer);
//            TRACE_ERR(msgbuffer);
//
//            result = errno;
//        }
//
//        g_DataService_signaldata.pFunctCB = 0;
//    }
//    if( SUCCESS == result )
//    {
//        result = libmessage_client_register_signal(&g_DataService_signaldata);
//    }
//
//
//    if( SUCCESS == result )
//    {
//        result = libmessage_svc_client_getdata(&g_DataService_signaldata);
//    }
//
//    if( SUCCESS == result )
//    {
//        if(SUCCESS != g_DataService_signaldata.response.result)
//        {
//            TRACE_ERR(" service error = %d %s",
//                    g_DataService_signaldata.response.result,
//                    strerror(g_DataService_signaldata.response.result));
//
//        }
//        result = g_DataService_signaldata.response.result;
//    }
//    else
//    {
//    }
//
//    //*********************************************************
//    //  create thread for callback
//    //*********************************************************
//    if( SUCCESS == result )
//    {
//
//    }

    return result ;
}
