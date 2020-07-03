/*
 ============================================================================
 Name        : cli_message.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */


//#include <limits.h>
//#include <unistd.h>
//
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <fcntl.h>
//
///* D'après POSIX.1-2001 */
//#include <sys/select.h>
//#include <poll.h>
//#include <semaphore.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "libmessage.h"

// sem_t *g_pSemGedate = 0;


//int check_fifo(const char* a_argv1)
//{
//    int result = 0;
//    char    vClientName[NAME_MAX] = {0};
//    int     fdServer = -1;
//    char    buffer[1024] = {0};
//
//    struct pollfd   vPollfdClient = {0};
//    nfds_t          vNfds    = 1;
//    int             vTimeout = 500;
//
//    vPollfdClient.fd      = -1;
//
//    //*********************************************************
//    //          create client fifo
//    //*********************************************************
//    snprintf(vClientName,NAME_MAX-1,"/tmp/client.%s",a_argv1);
//
//    result = unlink(vClientName);
//
//    result = libmessage_mkfifo(vClientName);
//
//    //*********************************************************
//    //          open client endpoint
//    //*********************************************************
//    if( 0 == result )
//    {
//        result = libmessage_openfifo(vClientName,O_RDONLY,&vPollfdClient.fd );
//    }
//
//    if( 0 == result )
//    {
//        do
//        {
////            printf("type any key to continue \n");
////            getchar();
//            if( -1 != fdServer )
//                close(fdServer);
//            //*********************************************************
//            //          open server fifo
//            //*********************************************************
//            // open server endpoint  argv[1] WO
//            fdServer = -1;
//            result = libmessage_openfifo(SVR_TIME_GETDATE,O_WRONLY,&fdServer);
//
//            //*********************************************************
//            //          write request
//            //*********************************************************
//            //send request to server endpoint
//            errno = 0;
//            result = write(fdServer,vClientName,strlen(vClientName));
//            if(-1 ==  result)
//            {
//                printf("Error %d: write(-%s-) %s \n",
//                        errno,SVR_TIME_GETDATE,strerror(errno));
//            }
//            else
//            {
//                printf("server write(%s) result=%d \n",vClientName, result);
//            }
//            //*****************
//            if( 0 < result )
//            {
//                //vPollfdClient.fd = vPollfdClient.fd;
//                vPollfdClient.events = POLLIN | POLLPRI ;
//                vPollfdClient.revents = 0;
//
//                // flush
//                errno=0;
//                result = ftruncate(vPollfdClient.fd,0);
//                printf("flush ftruncate : ret=%d errno=%d %s \n",result,errno,strerror(errno));
//
//                memset(buffer,0,sizeof(buffer));
//                errno=0;
//                result = read(vPollfdClient.fd,buffer,1024);
//                printf("flush read : ret=%d errno=%d %s \n",result,errno,strerror(errno));
//
//                errno = 0;
//                result  = poll(&vPollfdClient, vNfds, vTimeout);
//
//                printf("poll  result=%d: revents=%d 0x%X \n",
//                        result, (int)vPollfdClient.revents,(int)vPollfdClient.revents);
//
//
//                if( ( -1 == result ) )
//                {
//                    printf("Error:  poll() errno=%d %s \n",errno,strerror(errno));
//                    result = errno;
//                    close(fdServer);
//                    fdServer = -1;
//                }
//                else if( (result ) && ( vPollfdClient.revents & POLLHUP) )
//                {
//                    printf("Error:  poll() POLLHUP event \n");
//
//                    result = EPIPE;
//                    close(fdServer);
//                    fdServer = -1;
//                }
//                else if( 0 == result )
//                {
//                    printf("Error Timeout %d: poll() %s \n",errno,strerror(errno));
//                    result = errno;
//                    close(fdServer);
//                    fdServer = -1;
//                }
//                else
//                {
//                    //                    printf("poll  result=%d: revents=%d %X\n",
//                    //                            result, (int)vPollfd.revents,(int)vPollfd.revents);
//                    result = errno;
//
//                    //*********************************************************
//                    //          read response from server
//                    //*********************************************************
//                    do
//                    {
//                        memset(buffer,0,sizeof(buffer));
//                        errno = 0;
//                        result = read(vPollfdClient.fd,buffer,1024);
//
//                        if( 0 == result )
//                        {
//                            printf("Error client %d: read(-%s-) size == 0  \n",
//                                    errno,vClientName);
//                        }
//                        else if (-1 == result )
//                        {
//                            printf("Error client  %d: read(-%s-) %s \n",
//                                    errno,vClientName,strerror(errno));
//                        }
//                        else
//                        {
//                            printf("client read server response = %s  size=%d \n",buffer,result);
//                        }
//                    }while( 0 == result );
//                }
//            }
//
//        }while(1);
//
//        //close  server endpoint
//        close(fdServer);
//    }
//
//    // read response from server
//
//    return result;
//}


int main(int argc, char *argv[])
{
    int     result = EXIT_SUCCESS;
    double  vDate = 0.0;
    int ii = 0;


    if( argc > 1 )
    {
        do{
            if( ii == 10 )
            {
                ii = 0;
                printf("%s type any key to continue \n",getStrDate());
                getchar();
            }
            else
            {
                ii++;
            }

            result = libmessage_getdate(argv[1],&vDate);

        }while(1);
    }
    else
    {
        printf("Error no client name ! \n");
    }

    printf("exit result=%d ii=%d \n",result, ii);

    return result;
}
