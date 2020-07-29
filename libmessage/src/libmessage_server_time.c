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

static sDataThreadCtx_t g_TheadCtx_getdate  = {0};
//static sDataThreadCtx_t g_TheadCtx_setdate= {0};
//static sDataThreadCtx_t g_TheadCtx_Signaldate= {0};


//************************************************************
//  client side
//      _OUT_ double *a_pDate : buffer data output
//      return:
//          SUCCESS    0
//          EINVAL          22  Invalid argument
//************************************************************
int libmsg_srvtime_getdate( _IN_ const char* a_UniqID, _OUT_ double *a_Date)
{
    int     result                  = 0;
    char msgbuffer[APISYSLOG_MSG_SIZE] = {0};

    sRequest_t          request  = {0};
    sResponse_t         response = {0};
    sGetdateResponse_t  *pResponse  = 0;

    result = getMqClientname(a_UniqID,SVC_GETDATE,request.filenameClient);

    if( 0 == result )
    {
        result = libmsg_cli_getdata( SERVER_TIME_GETDATE,
                &request,&response);
    }

    if( 0 == result )
    {
        pResponse  = (sGetdateResponse_t  *)response.data ;
        *a_Date = getDateDouble(pResponse->timespesc);

        snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                " : date=%ld.%09ld *date=%.9f result=%d\n",
                pResponse->timespesc.tv_sec,
                pResponse->timespesc.tv_nsec,
                *a_Date,
                result);
        TRACE_LOG(msgbuffer);
    }

    return result;
}

//************************************************************
//*
//************************************************************
int libmsg_srvtime_register_getdate(libmsg_pFunctCB_t a_pFunctCB)
{
    int result = 0;

    //*****************************
    // prepare data thread
    //*****************************

    g_TheadCtx_getdate.dataService.pFunctCB = a_pFunctCB;

    strncpy(g_TheadCtx_getdate.dataService.filenameServer,
            SERVER_TIME_GETDATE,
            sizeof(g_TheadCtx_getdate.dataService.filenameServer)-1);

    strncpy(g_TheadCtx_getdate.dataService.filenameSemaphore,
            SERVER_TIME_GETDATE,strlen(SERVER_TIME_GETDATE)+1);

    result = libmsg_srv_register_svc(&g_TheadCtx_getdate);

    return result;
}
//************************************************************
//*
//************************************************************
int libmsg_srvtime_wait()
{
    int result = 0;

    result = pthread_join(g_TheadCtx_getdate.pthreadID,0);


    //*****************************
    // LIBMESSAGE_ID_END
    //*****************************

    // wait on end of thread
    return result;
}
