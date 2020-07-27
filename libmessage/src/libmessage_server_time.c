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
    char    vBufferIN[HARD_MAX]     = {0};
    uint32_t SizeBuffIn             = 0;
    char    vBufferOUT[HARD_MAX]    = {0};
    uint32_t SizeBuffOut            = sizeof(vBufferOUT);
    char    vClientName[NAME_MAX]   = {0};


    result = getMQname(a_UniqID,SVC_GETDATE,vClientName);


    strncpy(vBufferIN,vClientName,HARD_MAX);
    SizeBuffIn = strlen(vBufferIN);


    result = libmsg_cli_getdata( vClientName,SERVER_TIME_GETDATE,
            SizeBuffIn, vBufferIN,
            SizeBuffOut,vBufferOUT);

    if( 0 == result )
    {
        *a_Date = *((double*)vBufferOUT);
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

   //sDataThreadCtx_t    *pDataThreadCtx = getTheadCtx(eLIBMSG_ID_GETDATA);

    g_TheadCtx_getdate.dataService.pFunctCB = a_pFunctCB;

   strncpy(g_TheadCtx_getdate.dataService.filenameServer,
           SERVER_TIME_GETDATE,
           sizeof(g_TheadCtx_getdate.dataService.filenameServer)-1);


//   strncpy(g_TheadCtx_getdate.dataService.filenameSemaphore,
//           getNameService(eLIBMSG_ID_GETDATA,eLIBMSG_COL_SEM),NAME_MAX-1);

    result = libmsg_server_register_svc(&g_TheadCtx_getdate);

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
