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


 sem_t *g_pSemGedate = 0;


static int libmessage_cbfcnt_getdate(char* a_pData)
{
    int result = 0;
    char outstr[200];
     time_t t;
     struct tm *tmp;

     t = time(NULL);
     tmp = localtime(&t);

     strftime(outstr, sizeof(outstr), "%a, %d %b %Y %T %z", tmp) ;


     strncpy(a_pData,outstr,LIBMESSAGE_MAX_BUFFER-1);

    printf("libmessage_cbfcnt_getdate: date=%s\n",a_pData);

    return result;
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
    char    buffer[1024] = {0};
    struct pollfd   vPollfd = {0};
    nfds_t          vNfds    = 1;
    int             vTimeout = -1;

    int ii = 0;



    result = libmessage_mkfifo(SVR_TIME_GETDATE);

    //*********************************************************
    //          open server endpoint
    //*********************************************************

    if( 0 == result )
    {
        // open server endpoint
        errno = 0;
        fdServer = open(SVR_TIME_GETDATE,O_NONBLOCK|O_CLOEXEC|O_RDONLY);

        if( -1 == fdServer )
        {
            printf("%s _1_ open(-%s-) err=%d %s \n",
                    getStrDate(),SVR_TIME_GETDATE,errno,strerror(errno));
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
                vPollfd.fd = fdServer;
                vPollfd.events = POLLIN | POLLPRI ;
                vPollfd.revents = 0;
                 //***************************************************
                //              open
                //***************************************************
                close(fdServer);
                errno = 0;
                fdServer = open(SVR_TIME_GETDATE,O_NONBLOCK|O_CLOEXEC|O_RDONLY);  // File exists

                printf("%s _2_ poll POLLHUP errno=%d: open_2(-%s-) %s fdServer=%d\n",
                        getStrDate(),errno,
                        SVR_TIME_GETDATE,strerror(errno),fdServer);

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
                errno = 0;
                result  = poll(&vPollfd, vNfds, vTimeout);

                printf("%s _4_ poll  result=%d: revents=%d 0x%X \n",
                        getStrDate(),result, (int)vPollfd.revents,(int)vPollfd.revents);

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
//                    fdServer = open(SVR_TIME_GETDATE,O_NONBLOCK|O_CLOEXEC|O_RDONLY);  // File exists
//
//                    printf("%s _4_ poll POLLHUP errno=%d: open_2(-%s-) %s fdServer=%d\n",
//                            getStrDate(),errno,
//                            SVR_TIME_GETDATE,strerror(errno),fdServer);
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
                            getStrDate(),SVR_TIME_GETDATE,errno,strerror(errno));
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


//    result =  libmessage_register_service(
//            LIBMESSAGE_SRVID_TIME,
//            SERVER_TIME_ID_GETDATE ,
//            &libmessage_cbfcnt_getdate);

//    result = libmessage_register_service_time( SERVER_TIME_SETDATE, libmessage_cbfcnt_setdate);
//    result = libmessage_register_service_time( SERVER_TIME_SIGNAL,  libmessage_cbfcnt_signal,     libmessage_cbfcnt_signal);

//    result = libmessage_server_wait();


    sem_unlink(SVR_TIME_GETDATE_SEM);
    errno = 0;
    g_pSemGedate = sem_open(SVR_TIME_GETDATE_SEM,
            O_CREAT,
            S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO,/* 07777 */
            //S_IRWXU | S_IRWXG |S_IRWXO
            1);
    fprintf(stderr,"sem_open(%s) result=0x%p errno=%d %s \n",
            SVR_TIME_GETDATE_SEM,(void*)g_pSemGedate,
            errno,strerror(errno));


    result = srv_getdate();

    //result = libmessage_srvtime_init();

//    if( 0 == result )
//    {
//        result = srv_getdate();
//    }
//    else
//    {
//        // error
//       fprintf(stderr,"libmessage_srvtime_init Error : result=%d errno=%d %s \n",
//                result, errno, strerror(errno));
//    }

//    result = libmessage_srvtime_init();
//    result = libmessage_server_wait();

    return result;
}

