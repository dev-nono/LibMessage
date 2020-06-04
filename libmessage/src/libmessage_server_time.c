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


// #define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <limits.h>

// libmessage.h
#include <limits.h> // libmessage.h

#include "libmessage.h"



int server_time_getdate(const char* a_Caller, double *a_Date)
{
    int             result  = 0;
    struct mq_attr  vAttr   = {0};
    mqd_t           vFdServer_getdate   = 0;
    mqd_t           vFdPidClient        = 0;
    char            vPidClientName[NAME_MAX+1] = {0};
    ssize_t         vLenReceive                = 0;
    char            vBuffer[sizeof(double)];
    pid_t           vTid = syscall(SYS_gettid);



    vAttr.mq_flags =     O_CLOEXEC;

    snprintf(vPidClientName,NAME_MAX,"/%s.%d",a_Caller,vTid);

    *a_Date = 0.0;

    printf("libmessage_server_time_getdate %s %s\n",
            SERVER_TIME,
            SERVER_TIME_GETDATE);

    //********************************
    // open mq for request get_date
    //********************************

    vFdServer_getdate =  mq_open(SERVER_TIME_GETDATE, O_CREAT,S_IRWXG,&vAttr);

    if( vFdServer_getdate  == ( (mqd_t)(-1) ))
    {   //  error
        result = errno;
        printf("server_time_getdate: mq_open(%s) error %d  %s",
                SERVER_TIME_GETDATE,result,strerror(result));
    }

    //********************************
    // open mq for response
    //********************************
    if( 0 == result )
    {
        vFdPidClient =  mq_open(vPidClientName, O_CREAT,S_IRWXG,&vAttr);

        if( ( (mqd_t)(-1) ) == vFdPidClient)
        {   //  error
            result = errno;
            printf("server_time_getdate: mq_open(%s) error %d  %s",
                    vPidClientName,result,strerror(result));
        }
    }
    //********************************
    // send request getdate
    //********************************
    if( 0 == result )
    {
        result = mq_send(vFdServer_getdate, vPidClientName,NAME_MAX, 0U);

        if( 0 != result )
        {   //  error
            result = errno;
            printf("server_time_getdate: mq_send(%s) error %d  %s",
                    SERVER_TIME_GETDATE,result,strerror(result));
        }
    }

    //********************************
    // wait receive date
    //********************************
    if( 0 == result )
    {
        vLenReceive =  mq_receive(vFdPidClient,
                vBuffer,
                sizeof(vBuffer),
                0U);

        if( (-1) == vLenReceive )
        {
            result = errno;
            printf("server_time_getdate: mq_receive(%s) error %d  %s\n",
                    vPidClientName,result,strerror(result));

        }
        if( sizeof(vBuffer)    != vLenReceive )
        {
            printf("server_time_getdate: mq_receive(%lu,%s) error vLenReceive=%ld\n",
                    sizeof(vBuffer),vPidClientName,vLenReceive);
            result = EMSGSIZE;
        }
        if( sizeof(vBuffer) == vLenReceive )
        {
            *a_Date =  *((double*)&vBuffer);
        }
    }

    mq_close(vFdServer_getdate);
    mq_close(vFdPidClient);
    mq_unlink(vPidClientName);

    return result;
}
