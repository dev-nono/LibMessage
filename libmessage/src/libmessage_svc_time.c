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



static sDataThreadCtx_t g_ThdCtxGetdata = {0};
//static sDataThreadCtx_t g_ThdCtxSetdata = {0};
//static sDataThreadCtx_t g_ThdCtxSignaldata = {0};


//************************************************************
//*
//************************************************************
int libmessage_srvtime_wait()
{
    TRACE_IN("_IN")
    int result = 0;

    //broadcast signal start

    // joint thread 1

    pthread_join(g_ThdCtxGetdata.pthreadID,0);
    // joint thread 2
    // joint thread 3

    TRACE_OUT("_OUT result=%d",result)

    return result;
}
//************************************************************
//*
//************************************************************
int libmessage_srvtime_register_getdate(libmessage_pFunctCB_t a_pFunctCB)
{
    int result = 0;

    char msgbuffer[APISYSLOG_MSG_SIZE] = {0};

    memset(&g_ThdCtxGetdata,0,sizeof(g_ThdCtxGetdata));

    //*****************************
    // prepare data thread
    //*****************************
    g_ThdCtxGetdata.dataService.pFunctCB = a_pFunctCB;

    strncpy(g_ThdCtxGetdata.dataService.filenameServer,
            FILENAME_SVC_TIME_GETDATE,
            sizeof(g_ThdCtxGetdata.dataService.filenameServer)-1);

    //**************************************************
    //*  create semaphore
    //**************************************************
    if( 0 == result )
    {
        errno = 0;
        result = sem_unlink(SVR_TIME_GETDATE_SEM);
        if( 0 != result )
        {
            snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                    ": sem_unlink(%s) result=%d errno=%d %s",
                     SVR_TIME_GETDATE_SEM,
                    result,errno,strerror(errno));

            fprintf(stderr,"%s : %s",__FUNCTION__, msgbuffer);
            TRACE_ERR(msgbuffer);
        }

        result = 0;
        errno = 0;
        g_ThdCtxGetdata.dataService.pSemsvc = sem_open(SVR_TIME_GETDATE_SEM,
                O_CREAT,S_IRWXU,1U);
        if( SEM_FAILED == g_ThdCtxGetdata.dataService.pSemsvc)
        {
            snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                    "sem_open(%s) result=0x%p errno=%d %s",
                    SVR_TIME_GETDATE_SEM,
                    (void*)g_ThdCtxGetdata.dataService.pSemsvc,
                    errno,strerror(errno));

            fprintf(stderr,"%s : %s\n",__FUNCTION__, msgbuffer);
            TRACE_ERR(msgbuffer);
        }
    }

    if( 0 == result )
    {
       //*****************************
        // create new tread for listening incomming messages
        //*****************************
        errno = 0;
        result =  pthread_create(&g_ThdCtxGetdata.pthreadID,
                NULL,
                &libmessage_threadFunction_srv,
                (void*)&g_ThdCtxGetdata);

        if( 0 != result )
        {

            snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                    ": pthread_create() error =%d %s",
                    result,strerror(result));

            fprintf(stderr,"%s : %s",__FUNCTION__, msgbuffer);
            TRACE_ERR(msgbuffer);
        }
    }

    return result;
}
//************************************************************
//  client side
//      return:
//          EXIT_SUCCESS    0
//          EINVAL          22  Invalid argument
//************************************************************
int libmessage_getdate( const char *a_Callername,
                        double     *a_pDate)
{
    int result = EXIT_SUCCESS;
    char msgbuffer[APISYSLOG_MSG_SIZE] = {0};

    sDataService_t vDataService = {0};

    if( (!a_Callername) || (!*a_Callername) || (!a_pDate) )
    {
        result = EINVAL ;

        snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                " : Invalid argument error =%d %s",

                result,strerror(result));

        fprintf(stderr,"%s : %s",__FUNCTION__, msgbuffer);
        TRACE_ERR(msgbuffer);
    }

    if( EXIT_SUCCESS == result )
    {
        strcpy(vDataService.filenameClient,a_Callername);

        strcpy(vDataService.filenameServer,FILENAME_SVC_TIME_GETDATE);

        errno = 0;
        vDataService.pSemsvc = sem_open(SVR_TIME_GETDATE_SEM,0);

        if( SEM_FAILED == vDataService.pSemsvc)
        {
            snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                    ": sem_open(%s) result=0x%p errno=%d %s",
                    FILENAME_SVC_TIME_GETDATE,
                    (void*)vDataService.pSemsvc,
                    errno,strerror(errno));
            fprintf(stderr,"%s : %s",__FUNCTION__, msgbuffer);
            TRACE_ERR(msgbuffer);

            result = errno;
        }

        vDataService.pFunctCB = 0;
    }

    if( EXIT_SUCCESS == result )
    {
        result = libmessage_svc_getdata(&vDataService);
    }

    if( 0 < result )
    {
        struct timespec tp = {0};

        memcpy(&tp,vDataService.databuffer,sizeof(tp));

        *a_pDate = ((double)tp.tv_sec) + ((double)tp.tv_nsec*1e-9);
    }

    return result !=0;
}

