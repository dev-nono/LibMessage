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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* D'après POSIX.1-2001 */
#include <sys/select.h>

#include "libmessage_int.h"


#include "libmessage.h"

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
    char    vClientName[NAME_MAX] = {0};
    int     fdServer = -1;
    int     fdClient = -1;
    char    buffer[1024] = {0};


    //*********************************************************
    // create server endpoint
    //*********************************************************
    errno = 0;
    result = mkfifo(SVR_TIME_GETDATE,S_IRWXU);

    if( (0 != result ) && (EEXIST != errno) )
    {
        // error
        printf("Error %d: mkfifo(-%s-) %s \n",
                errno,SVR_TIME_GETDATE,strerror(errno));
    }
    else
    {
        printf("server name:  -%s- \n",SVR_TIME_GETDATE);
        result = 0;
    }
    //*********************************************************
    //          open server endpoint
    //*********************************************************

    if( 0 == result )
    {
        // open server endpoint
        errno = 0;
//        fdServer = open(SVR_TIME_GETDATE,S_IRWXU, O_NONBLOCK|O_CLOEXEC|O_RDONLY); // block
//        fdServer = open(SVR_TIME_GETDATE,S_IRWXU, O_NONBLOCK|O_CLOEXEC);  // File exists
        fdServer = open(SVR_TIME_GETDATE,O_NONBLOCK|O_CLOEXEC|O_RDONLY);  // File exists

        if( -1 == fdServer )
        {
            printf("Error %d: open(-%s-) %s \n",
                    errno,SVR_TIME_GETDATE,strerror(errno));
            result = errno;
        }
    }


    if( 0 == result )
    {
        int nfds = 0;
        fd_set readfds = {0};
        //fd_set writefds= {0};
        //fd_set *exceptfds,
        //struct timeval *timeout

        do
        {
            memset(vClientName,0,sizeof(vClientName));

//            result =  select(int nfds, fd_set *readfds, fd_set *writefds,
//                              fd_set *exceptfds, struct timeval *timeout);

            nfds = fdServer+1;
            printf("waiting incomming request \n");
            FD_ZERO(&readfds);
            FD_SET(fdServer, &readfds);


            errno = 0;
            result =  select( nfds, &readfds,0,0,0);

            if( -1 == result )
            {
                printf("Error %d: select() %s \n",errno,strerror(errno));
                result = errno;
            }
            else
            {
                //*********************************************************
                //          read request
                //*********************************************************

                errno = 0;
                result = read(fdServer,vClientName,1024);

                if( 0 == result )
                {
                    printf("Error %d: read(-%s-) size == 0  \n",
                            errno,vClientName);
                    result = -1;
                }
                else if (-1 == result )
                {
                    printf("Error %d: read(-%s-) %s \n",
                            errno,vClientName,strerror(errno));
                }
                else
                {
                    printf("server response = %s  size=%d \n",vClientName,result);
                }
            }

            //*********************************************************
            //          open client endpoint
            //*********************************************************
            if( 0 < result )
            {
                // open client endpoint  argv[1] WO
                errno = 0;
                fdClient = open(vClientName,O_NONBLOCK|O_CLOEXEC|O_WRONLY);

                if( -1 == fdClient  )
                {
                    printf("Error %d: open(-%s-) %s \n",
                            errno,vClientName,strerror(errno));
                    result = errno;
                }
            }


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
                printf("Error %d: write(-%s-) %s \n",
                        errno,SVR_TIME_GETDATE,strerror(errno));
            }
            else
            {
                printf("write(%s) ok len=%d \n",buffer,vsize);
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



    result = srv_getdate();

    return result;
}

