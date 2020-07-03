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


// #define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <limits.h>

#include "libmessage_common.h"
#include "libmessage_int.h"
#include "libmessage.h"



static char g_arrayServiceName[][NAME_MAX] =
{
        {SVCNAME_TIME_GETDATE},
        {SVCNAME_TIME_SETDATE},
        {SVCNAME_TIME_SIGNAL},
        {SVCNAME_TIME_END}
};


//const char* get_arrayServiceName(uint32_t a_ServiceID )
//{
//    char *serviceName = 0;
//
//    if( LIBMESSAGE_SVCID_TIME_END < a_ServiceID )
//    {
//        serviceName = g_arrayServiceName[a_ServiceID] ;
//    }
//
//    return serviceName;
//}

//************************************************************
//*
//************************************************************
int libmessage_srvtime_register_getdate()
{
    int result = 0;

    result = libmessage_mkfifo(SVCNAME_TIME_GETDATE);




    return result;
}
//************************************************************
//* return:
//      EXIT_SUCCESS    0
//      EINVAL          22  Invalid argument
//************************************************************
int libmessage_getdate( const char *a_Callername,
                        double     *a_Date)
{
    int result = EXIT_SUCCESS;

    sDataService_t vDataService = {0};

    if( (!a_Callername) || (!*a_Callername) || (!a_Date) )
    {
        result = EINVAL ;
    }

    if( EXIT_SUCCESS == result )
    {
        strncpy(vDataService.filenameClient,
                a_Callername,
                sizeof(vDataService.filenameClient)-1);

        strncpy(vDataService.filenameServer,
                SVCNAME_TIME_GETDATE,
                sizeof(vDataService.filenameServer)-1);


        errno = 0;
        vDataService.pSemsvc = sem_open(SVR_TIME_GETDATE_SEM,0);
        fprintf(stderr,"sem_open(%s) result=0x%p errno=%d %s ",
                SVR_TIME_GETDATE_SEM,(void*)vDataService.pSemsvc,
                errno,strerror(errno));

        if( SEM_FAILED == vDataService.pSemsvc)
        {
            result = errno;
        }

        vDataService.pFuncCB = 0;
    }

    if( EXIT_SUCCESS == result )
    {
        result = libmessage_svc_getdata(&vDataService);
    }

    return result;
}


//************************************************************
//*
//************************************************************
//int libmessage_getdate( const char* a_Callername,
//                        //const char* a_Servername,
//                        uint32_t         a_ServiceID,
//                        double *a_Date)
//{
//    int             result  = 0;
//    struct mq_attr  vAttr   = {0};
//    mqd_t           vFdServer_getdate   = -1;
//    mqd_t           vFdPidClient        = -1;
//    char            vPidClientName[NAME_MAX+1] = {0};
//    ssize_t         vLenReceive                = 0;
//    char            vBuffer[sizeof(double)];
//    pid_t           vTid = syscall(SYS_gettid);
//
//    vAttr.mq_flags =     O_CLOEXEC;
//
//    if( (0 == a_Callername) || (0 == (a_Date) ) )
//    {
//        result = EINVAL;
//    }
//
//    if( 0 == result )
//    {
//        snprintf(vPidClientName,NAME_MAX,"/%s.%d",a_Callername,vTid);
//        *a_Date = 0.0;
//
//        vAttr.mq_flags  = O_CLOEXEC;
//        vAttr.mq_curmsgs = 9;
//        vAttr.mq_maxmsg = 9;
//        vAttr.mq_msgsize = 1024;
//        errno           = 0;
//
//        //********************************
//        // open mq server
//        //********************************
//        vFdServer_getdate =  mq_open(get_arrayServiceName(a_ServiceID),
//                O_RDWR,S_IRWXO | S_IRWXG | S_IRWXU,&vAttr);
//
//        if( vFdServer_getdate  == ( (mqd_t)(-1) ))
//        {   //  error
//            result = errno;
//            printf("libmessage_getdate: mq_open(%s) error %d  %s",
//                    get_arrayServiceName(a_ServiceID),result,strerror(result));
//        }
//    }
//    //********************************
//    // open mq for response
//    //********************************
//    if( 0 == result )
//    {
//        vAttr.mq_flags  = O_CLOEXEC;
//        vAttr.mq_curmsgs = 9;
//        vAttr.mq_maxmsg = 9;
//        vAttr.mq_msgsize = 1024;
//        errno           = 0;
//
//        vFdPidClient =  mq_open(vPidClientName,
//                O_CREAT,S_IRWXO | S_IRWXG | S_IRWXU ,&vAttr);
//
//        if( ( (mqd_t)(-1) ) == vFdPidClient)
//        {   //  error
//            result = errno;
//            printf("libmessage_getdate: mq_open(%s) error %d  %s",
//                    vPidClientName,result,strerror(result));
//        }
//    }
//    //********************************
//    // send request getdate
//    //********************************
//    if( 0 == result )
//    {
//        result = mq_send(vFdServer_getdate, "0",2, 0U);
//
//        if( -1 == result )
//        {   //  error
//            result = errno;
//            printf("libmessage_getdate: mq_send(%s) error %d  %s",
//                    get_arrayServiceName(a_ServiceID),result,strerror(result));
//        }
//    }
//
//    //********************************
//    // wait receive date
//    //********************************
//    if( 0 == result )
//    {
//        vLenReceive =  mq_receive(vFdPidClient,
//                vBuffer,
//                sizeof(vBuffer),
//                0U);
//
//        if( (-1) == vLenReceive )
//        {
//            result = errno;
//            printf("libmessage_getdate: mq_receive(%s) error %d  %s\n",
//                    vPidClientName,result,strerror(result));
//        }
//        if( sizeof(vBuffer)    != vLenReceive )
//        {
//            printf("libmessage_getdate: mq_receive(%lu,%s) error vLenReceive=%ld\n",
//                    sizeof(vBuffer),vPidClientName,vLenReceive);
//            result = EMSGSIZE;
//        }
//        if( sizeof(vBuffer) == vLenReceive )
//        {
//            *a_Date =  *((double*)&vBuffer);
//        }
//    }
//    if( (-1) != vFdServer_getdate )
//        mq_close(vFdServer_getdate);
//    if( (-1) != vFdPidClient )
//        mq_close(vFdPidClient);
//    if ( 0 != (*vPidClientName) )
//            mq_unlink(vPidClientName);
//
//    return result;
//}
