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

    result = libmessage_srvtime_register_svc(eLIBMSG_ID_GETDATA, a_pFunctCB);

    return result;
}
//************************************************************
//*
//************************************************************
int libmessage_srvtime_register_setdate(libmessage_pFunctCB_t a_pFunctCB)
{
    int result = 0;

    result = libmessage_srvtime_register_svc(eLIBMSG_ID_SETDATA,
            a_pFunctCB);

    return result;
}
//************************************************************
//*
//************************************************************
int libmessage_srvtime_register_signaldate(libmessage_pFunctCB_t a_pFunctCB)
{
    int result = 0;

    result = libmessage_srvtime_register_svc(eLIBMSG_ID_SIGNAL,
            a_pFunctCB);

    return result;
}


//************************************************************
//  client side
//      return:
//          SUCCESS    0
//          EINVAL          22  Invalid argument
//************************************************************
int libmessage_getdate(
        _IN_  const char *a_Callername,   // address fifo name to respond
        _OUT_ double     *a_pDate)        // buffer data output
{
    int         result                          = SUCCESS;
    char        msgbuffer[APISYSLOG_MSG_SIZE]   = {0};

    sDataService_t vDataService = {0};

    if( (!a_Callername) || (!*a_Callername) || (!a_pDate) )
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
        strcpy(vDataService.filenameClientSuffix,a_Callername);

        strcpy(vDataService.filenameServer,getNameService(eLIBMSG_ID_GETDATA,eLIBMSG_COL_SRV_FILENAME));

        errno = 0;
        vDataService.pSemsvc = sem_open(getNameService(eLIBMSG_ID_GETDATA,eLIBMSG_COL_SEM),0);

        if( SEM_FAILED == vDataService.pSemsvc)
        {
            snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                    ": sem_open(%s) result=0x%p errno=%d %s",
                    getNameService(eLIBMSG_ID_GETDATA,eLIBMSG_COL_SRV_FILENAME),
                    (void*)vDataService.pSemsvc,
                    errno,strerror(errno));
            fprintf(stderr,"%s : %s \n",__FUNCTION__, msgbuffer);
            TRACE_ERR(msgbuffer);

            result = errno;
        }

        vDataService.pFunctCB = 0;
    }

    if( SUCCESS == result )
    {
        result = libmessage_svc_getdata(&vDataService);
    }

    if( 0 < result )
    {
        *a_pDate = (    (double)vDataService.response.uResponse.getdate.timespesc.tv_sec)
                + ((double)vDataService.response.uResponse.getdate.timespesc.tv_nsec*1e-9);

        TRACE_DBG1("%s : getdate = %ld.%09ld",
                __FUNCTION__,
                vDataService.response.uResponse.getdate.timespesc.tv_sec,
                vDataService.response.uResponse.getdate.timespesc.tv_nsec);
    }

    return result !=0;
}


//************************************************************
//  client side
//      return:
//          SUCCESS    0
//          EINVAL          22  Invalid argument
//************************************************************
int libmessage_setdate(
        _IN_  const char *a_Callername,   // address fifo name to respond
        _IN_    double     a_pDate)        // buffer data output
{
    int result = SUCCESS;

    char msgbuffer[APISYSLOG_MSG_SIZE] = {0};

    sDataService_t vDataService = {0};

    if( (!a_Callername) || (!*a_Callername) || (!a_pDate) )
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
        strcpy(vDataService.filenameClientSuffix,a_Callername);

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

    if( SUCCESS == result )
    {
        result = libmessage_svc_getdata(&vDataService);
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

    return result ;
}
