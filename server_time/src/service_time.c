/*
 ============================================================================
 Name        : server_time.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <limits.h>
#include <mqueue.h>
#include <time.h>
#include <poll.h>

#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* D'après POSIX.1-2001 */
#include <sys/select.h>
#include <poll.h>
#include <semaphore.h>

#include "apisyslog.h"
#include "utils.h"
#include "libmessage_int.h"
#include "libmessage.h"

#include "libmessage_svc_time.h"


sem_t *g_pSemGedate = 0;


// static int libmessage_cbfcnt_signaldate(char* a_pData)
// {
//     int result = 0;
//
//     return  result;
// }
//
// static int libmessage_cbfcnt_setdate(char* a_pData)
// {
//     int result = 0;
//
//     return  result;
// }

static int libmessage_cbfcnt_getdate(char* a_pData)
{
    int result = 0;
    char msgbuffer[APISYSLOG_MSG_SIZE] = {0};

    struct timespec tp = {0};

    clock_gettime(CLOCK_MONOTONIC_RAW, &tp);

    //    result = snprintf(a_pData,PIPE_BUF-10,"%lld.%.9ld",
    //            (long long)tp.tv_sec,tp.tv_nsec);

    memcpy(a_pData,&tp,sizeof(tp));

    result =  sizeof(tp);

    snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
            " : date=%ld.%09ld len=%d\n",
            tp.tv_sec,tp.tv_nsec,result);

    TRACE_LOG(msgbuffer);

    return result;
}

//*********************************************************
//*
//*********************************************************
int main(int argc, char *argv[])
{
    int result = 0;

    (void)argv;
    (void)argc;

    apisyslog_init("");

    //    result = srv_getdate();
    //
    //
    //
    result = libmessage_srvtime_register_getdate(&libmessage_cbfcnt_getdate);
    //    result = libmessage_srvtime_register_setdate(&libmessage_cbfcnt_setdate);
    //    result = libmessage_srvtime_register_signaldate(&libmessage_cbfcnt_signaldate);


    result = libmessage_srvtime_wait();


    return result;
}

