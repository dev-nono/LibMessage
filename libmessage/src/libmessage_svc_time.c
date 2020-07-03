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

#include "libmessage_int.h"
#include "libmessage.h"



static char g_arrayServiceName[][NAME_MAX] =
{
        {SVCNAME_TIME_GETDATE},
        {SVCNAME_TIME_SETDATE},
        {SVCNAME_TIME_SIGNAL},
        {SVCNAME_TIME_END}
};


const char* get_arrayServiceName(uint32_t a_ServiceID )
{
    char *serviceName = 0;

    if( LIBMESSAGE_SVCID_TIME_END < a_ServiceID )
    {
        serviceName = g_arrayServiceName[a_ServiceID] ;
    }

    return serviceName;
}

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
//*
//************************************************************
int libmessage_getdate( const char *a_Callername,
                        //const char *a_Servername,
        uint32_t         a_ServiceID,
                        double     *a_Date,
                        sem_t *a_pSemGedate)
{
    int result = 0;
    int result2 = 0;
    char    vClientName[NAME_MAX] = {0};
    int     fdServer = -1;
    char    buffer[1024] = {0};

    struct pollfd   vPollfdClient = {0};
    nfds_t          vNfds    = 1;
    int             vTimeout = 500;

    vPollfdClient.fd      = -1;

    //*********************************************************
    //          create client fifo
    //*********************************************************
    snprintf(vClientName,NAME_MAX-1,"/tmp/client.%s",a_Callername);

    //result = unlink(vClientName);

    result = libmessage_mkfifo(vClientName);

    //*********************************************************
    //          open client endpoint
    //*********************************************************
    if( 0 == result )
    {
        result = libmessage_openfifo(vClientName,O_RDONLY,&vPollfdClient.fd );
    }

    //*********************************************************
    //          open server fifo
    //*********************************************************
    if( 0 == result )
    {
        // open server endpoint  argv[1]
        fdServer = -1;
        result = libmessage_openfifo(SVR_TIME_GETDATE,O_WRONLY,&fdServer);
    }

    struct timespec abs_timeout = {0,1e9 / 100  };
    //***************************************************
    //              lock
    //***************************************************
    result = sem_timedwait(a_pSemGedate,&abs_timeout);
    printf("%s _1_ sem_wait() result=%d err=%d %s \n",
            getStrDate(),result,errno,strerror(errno));


    //*********************************************************
    //          write request
    //*********************************************************
    if( 0 == result )
    {
        //send request to server endpoint
        errno = 0;
        result = write(fdServer,vClientName,strlen(vClientName));

        if(-1 ==  result)
        {
            printf("%s _2_ server write(-%s-) Error %d %s \n",
                    getStrDate(),SVR_TIME_GETDATE,errno,strerror(errno));
            result = errno;
        }
        else
        {
            printf("%s _21_ server write(%s) result=%d \n",
                    getStrDate(),SVR_TIME_GETDATE, result);
            result = 0;
        }
            close(fdServer);
            fdServer = -1;
     result = sem_post(a_pSemGedate);
   }


    fprintf(stderr,"%s _3_ sem_post resulty=%d err=%d %s\n",
            getStrDate(),result,errno,strerror(errno));

//    printf("%s _3_ type any key to continue 1 \n",getStrDate());
//    getchar();

    //*********************************************************
    //      waiting receive response : polling
    //*********************************************************
    if( 0 == result )
    {
        vPollfdClient.events = POLLIN | POLLPRI ;
        vPollfdClient.revents = 0;
        vTimeout = -1;

        errno = 0;
        result  = poll(&vPollfdClient, vNfds, vTimeout);

        printf("%s _31_ poll  result=%d: revents=%d 0x%X \n",
                getStrDate(),result,
                (int)vPollfdClient.revents,(int)vPollfdClient.revents);


        if( ( -1 == result ) )
        {
            printf("%s _32_ poll() errno=%d %s \n",
                    getStrDate(),errno,strerror(errno));
            result = errno;
        }
        else if( (result ) && ( vPollfdClient.revents & POLLHUP) )
        {
            printf("%s _33_ poll() result=%d POLLHUP event \n",
                    getStrDate(),result);

            result = EPIPE;
        }
        else if( 0 == result )
        {
            printf("%s _34_ poll() Error Timeout %d %s \n",
                    getStrDate(),errno,strerror(errno));
            result = errno;
        }
        else
        {
            result = 0;
        }
    }

    //*********************************************************
    //          read response from server
    //*********************************************************
  if( 0 == result )
    {

        memset(buffer,0,sizeof(buffer));
        errno = 0;
        result = read(vPollfdClient.fd,buffer,1024);

        if( 0 == result )
        {
            printf("%s _4_ read(-%s-) Error client %d:  result == 0  \n",
                    getStrDate(),vClientName,result);
        }
        else if (-1 == result )
        {
            printf("%s _41_ read(-%s-) error client %d %s \n",
                    getStrDate(),vClientName,errno,strerror(errno));
        }
        else
        {
            printf("%s _42_ client read server response = %s  size=%d \n",
                    getStrDate(),buffer,result);
        }
    }

  if( -1 != fdServer)
      close(fdServer);

  if( -1 != vPollfdClient.fd)
      close(vPollfdClient.fd);

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
