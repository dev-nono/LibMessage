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

#include "libmessage_int.h"
#include "libmessage.h"

#include "libmessage_svc_time.h"


 sem_t *g_pSemGedate = 0;


 static int libmessage_cbfcnt_signaldate(char* a_pData)
 {
     int result = 0;

     return  result;
 }

 static int libmessage_cbfcnt_setdate(char* a_pData)
 {
     int result = 0;

     return  result;
 }

static int libmessage_cbfcnt_getdate(char* a_pData)
{
    int result = 0;
    //char outstr[PIPE_BUF] = {0};

    struct timespec tp = {0};

    clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
    int vsize = 0;

    vsize = snprintf(a_pData,PIPE_BUF-10,"%lld.%.9ld",
            (long long)tp.tv_sec,tp.tv_nsec);

     fprintf(stderr,"%s %s: date=%s len=%d\n",
             (char*)getStrDate(),__FUNCTION__,a_pData,vsize);


     return vsize;
}
//static int libmessage_cbfcnt_setdate(void* a_pData)
//{
//    int result = 0;
//
//    return result;
//}
//static int libmessage_cbfcnt_signal(void* a_pData)
//{
//    int result = 0;
//
//    return result;
//}


int srv_getdate()
{
    int result = 0;
    int result2 = 0;
    char    vClientName[NAME_MAX] = {0};
    int     fdServer = -1;
    int     fdClient = -1;
    char    buffer[PIPE_BUF] = {0};
    struct pollfd   vPollfd = {0};
    nfds_t          vNfds    = 1;
    int             vTimeout = -1;

    int ii = 0;


    result = libmessage_mkfifo(SVCNAME_TIME_GETDATE);

    //*********************************************************
    //          open server endpoint
    //*********************************************************

    if( 0 == result )
    {
        // open server endpoint
        errno = 0;
        fdServer = open(SVCNAME_TIME_GETDATE,O_NONBLOCK|O_CLOEXEC|O_RDONLY);

        if( -1 == fdServer )
        {
            printf("%s _1_ open(-%s-) err=%d %s \n",
                    getStrDate(),SVCNAME_TIME_GETDATE,errno,strerror(errno));
            result = errno;
        }
    }


struct timespec abs_timeout = {3,0  };
result = sem_wait(g_pSemGedate);
fprintf(stderr,"sem_wait() result=%d  errno=%d %s\n",
        result, errno,strerror(errno));

fprintf(stderr,"type to continue ... \n");
getchar();

    if( 0 == result )
    {

        do
        {
            memset(vClientName,0,sizeof(vClientName));

            vTimeout = -1;

            //do{
                 //***************************************************
                //              open
                //***************************************************
                close(fdServer);
                errno = 0;
                fdServer = open(SVCNAME_TIME_GETDATE,O_NONBLOCK|O_CLOEXEC|O_RDONLY);  // File exists

                fprintf(stderr,"%s _2_ poll POLLHUP errno=%d: open_2(-%s-) %s fdServer=%d\n",
                        getStrDate(),errno,
                        SVCNAME_TIME_GETDATE,strerror(errno),fdServer);

                //***************************************************
                //              unlock
                //***************************************************
                ////                result = lockf(fdServer, F_ULOCK, 0);
                //                    printf("%s _2_ lockf(F_ULOCK) err=%d %s \n",
                //                            getStrDate(),errno,strerror(errno));

                result = sem_post(g_pSemGedate);
                //result = sem_wait(g_pSemGedate);
                printf("%s _3_ sem_post() result=%d err=%d %s \n",
                        getStrDate(),result,errno,strerror(errno));

                //***************************************************
                //              poll
                //***************************************************
                vPollfd.fd = fdServer;
                vPollfd.events = POLLIN | POLLPRI ;
                vPollfd.revents = 0;
                errno = 0;

                result  = poll(&vPollfd, vNfds, vTimeout);

                printf("%s _4_ poll  result=%d: revents=%d 0x%X \n",
                        getStrDate(),result,
                        (int)vPollfd.revents,(int)vPollfd.revents);

                //***************************************************
                //              lock
                //***************************************************
                result = sem_wait(g_pSemGedate);
                printf("%s _5_ sem_wait() result=%d err=%d %s \n",
                        getStrDate(),result,errno,strerror(errno));


//                if( ( 0 < result  ) && (vPollfd.revents & POLLIN))
//                {
//                    //ok
//                    result = 0;
//                    break;
//                }
//                else  if( vPollfd.revents & POLLHUP )
//                {
//                    close(fdServer);
//                    errno = 0;
//                    fdServer = open(SVCNAME_TIME_GETDATE,O_NONBLOCK|O_CLOEXEC|O_RDONLY);  // File exists
//
//                    printf("%s _4_ poll POLLHUP errno=%d: open_2(-%s-) %s fdServer=%d\n",
//                            getStrDate(),errno,
//                            SVCNAME_TIME_GETDATE,strerror(errno),fdServer);
//
//                    if( -1 == fdServer )
//                    {
//                        result = errno;
//                    }
//                }
//                else
//                {
//                    break;
//                }

           // }while(1);

            if( 0 > result )
            {
                printf("%s _6_ poll() err=%d  %s \n",
                        getStrDate(),errno,strerror(errno));
                result = errno;
            }
            else
            {
                // read client request
                // client send filename for response
                memset(vClientName,0,sizeof(vClientName));
                errno = 0;
                result = read(fdServer,vClientName,1024);

                if( 0 == result )
                {
                    printf("%s _7_ read(-%s-) err=%d size == 0  \n",
                            getStrDate(),vClientName, errno);
                    result = -1;
                }
                else if (-1 == result )
                {
                    printf("%s _8_ read(-%s-) Error %d %s \n",
                            getStrDate(),vClientName,errno,strerror(errno));
                }
                else
                {
                    printf("%s _9_ read(-%s-) size=%d \n",
                            getStrDate(),vClientName,result);

                    //***************************************************
                    //              lock
                    //***************************************************
                    result2 = lockf(fdServer, F_LOCK, 0);
                    if (-1 == result2 )
                    {
                        printf("%s _10_ lockf(F_LOCK)fd=%d error=%d %s \n",
                                getStrDate(),fdServer,errno,strerror(errno));
                    }
                }
            }

            //*********************************************************
            //          open client endpoint
            //*********************************************************
            if( 0 < result )
            {
                // open client endpoint  argv[1] WO
                errno = 0;
                fdClient = open(vClientName,O_NONBLOCK|O_CLOEXEC|O_WRONLY); //

                if( -1 == fdClient  )
                {
                    printf("%s _11_ open(-%s-) err=%d %s \n",
                            getStrDate(),vClientName,errno,strerror(errno));
                    result = errno;
                }
            }

            if( 0 < result )
            {

                //*********************************************************
                //          write response
                //*********************************************************
                struct timespec tp = {0};
                clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
                int vsize = 0;

                vsize = snprintf(buffer,NAME_MAX,"%lld.%.9ld", (long long)tp.tv_sec,tp.tv_nsec);
                errno = 0;


                result = write(fdClient,buffer,vsize+1);
                if(-1 ==  result)
                {
                    printf("%s _12_ write(-%s-) err=%d %s \n",
                            getStrDate(),SVCNAME_TIME_GETDATE,errno,strerror(errno));
                }
                else
                {
                    printf("%s _13_ write(%s) ok len=%d \n",getStrDate(),buffer,vsize+1);
                }
            }

        }while(1);
    }

    return result;
}


//*********************************************************
//*
//*********************************************************
int main(int argc, char *argv[])
{
    int result = 0;

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

