/*
 ============================================================================
 Name        : cli_message.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
 #include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* D'après POSIX.1-2001 */
#include <sys/select.h>
#include <poll.h>


#include "libmessage.h"



int check_fifo(const char* a_argv1)
{
    int result = 0;
    char    vClientName[NAME_MAX] = {0};
    int     fdServer = -1;
    int     fdClient = -1;
    char    buffer[1024] = {0};

    int nfds = 0;
    fd_set readfds = {0};

    struct pollfd   vPollfd = {0};
    nfds_t          vNfds    = 1;
    int             vTimeout = -1;

    //*********************************************************
    //          create client fifo
    //*********************************************************
    snprintf(vClientName,NAME_MAX-1,"/tmp/client.%s",a_argv1);

    result = libmessage_mkfifo(vClientName);

    //*********************************************************
    //          open client endpoint
    //*********************************************************
    if( 0 == result )
    {
        // open client endpoint  argv[1] WO
        errno = 0;
//        fdClient = open(vClientName,S_IRWXU, O_NONBLOCK|O_CLOEXEC|O_RDONLY);
        fdClient = open(vClientName, O_NONBLOCK|O_CLOEXEC|O_RDONLY);

        if( -1 == fdClient  )
        {
            printf("Error client %d: open(-%s-) %s \n",
                    errno,vClientName,strerror(errno));
            result = errno;
        }
        else
        {
            printf("client open(-%s-) OK fd=%d \n",vClientName,fdClient);


//            memset(buffer,0,sizeof(buffer));
//            result = read(fdClient,buffer,1024);
//            printf("flush 1 = %s \n",buffer);

        }
    }


    if( 0 == result )
    {
        do
        {
//            do
//            {
//                errno = 0;
//                result = read(fdClient,buffer,1024);
//
//                if( 0 == result )
//                {
//                    printf("flush_1 Error result==0 errno=%d: read(-%s-) size == 0  \n",
//                            errno,vClientName);
//                }
//                else if (-1 == result )
//                {
//                    printf("flush_1 Error result == -1 , errno=%d: read(-%s-) %s \n",
//                            errno,vClientName,strerror(errno));
//                }
//                else
//                {
//                    printf("flush_1 read server response = %s  result=%d \n",buffer,result);
//                }
//
//                if (        ( result == 0 )
//                        ||  (( result == -1 ) && (errno == EAGAIN) )) //11
//                    break;
//            }while( 1);

            // res errno
            // 0   0    continue
            // -1   11  stop

            printf("type any key to continue \n");
            getchar();
            if( -1 != fdServer )
                close(fdServer);
            //*********************************************************
            //          open server fifo
            //*********************************************************
            // open server endpoint  argv[1] WO
            errno = 0;
            fdServer = open(SVR_TIME_GETDATE,O_NONBLOCK|O_CLOEXEC|O_WRONLY); //|O_CLOEXEC|O_WRONLY);

            if( -1 == fdServer )
            {
                printf("Error %d: open(-%s-) %s \n",
                        errno,SVR_TIME_GETDATE,strerror(errno));
                result = errno;
            }

            //*********************************************************
            //          write request
            //*********************************************************
            //send request to server endpoint
            errno = 0;
            result = write(fdServer,vClientName,strlen(vClientName)+1);
            if(-1 ==  result)
            {
                printf("Error %d: write(-%s-) %s \n",
                        errno,SVR_TIME_GETDATE,strerror(errno));
            }
            else
            {
                printf("server write(%s) result=%d \n",vClientName, result);
            }

            if( 0 < result )
            {
                vPollfd.fd = fdClient;
                vPollfd.events = POLLIN | POLLPRI ;
                vPollfd.revents = 0;

                do{

                    errno = 0;
                    result  = poll(&vPollfd, vNfds, vTimeout);

                    printf("poll  result=%d: revents=%d 0x%X \n",
                            result, (int)vPollfd.revents,(int)vPollfd.revents);

//                    if( (result ) && ( !(vPollfd.revents & POLLHUP)) )
//                        break;

                }while(0);

                if( 0 > result )
                {
                    printf("Error %d: poll() %s \n",errno,strerror(errno));
                    result = errno;
                }

//                nfds = fdClient+1;
//                FD_ZERO(&readfds);
//                FD_SET(fdClient, &readfds);
//
//                errno = 0;
//                result =  select( nfds, &readfds,0,0,0);
//
//                if( -1 == result )
//                {
//                    printf("Error %d: select() %s \n",errno,strerror(errno));
//                    result = errno;
//                }
                else
                {
//                    printf("poll  result=%d: revents=%d %X\n",
//                            result, (int)vPollfd.revents,(int)vPollfd.revents);
                    result = errno;

                    //*********************************************************
                    //          read response from server
                    //*********************************************************
                    do
                    {
                        errno = 0;
                        result = read(fdClient,buffer,1024);

                        if( 0 == result )
                        {
                            printf("Error client %d: read(-%s-) size == 0  \n",
                                    errno,vClientName);
                        }
                        else if (-1 == result )
                        {
                            printf("Error client  %d: read(-%s-) %s \n",
                                    errno,vClientName,strerror(errno));
                        }
                        else
                        {
                            printf("client read server response = %s  size=%d \n",buffer,result);
                        }
                    }while( 0 == result );
                }
            }

        }while(1);

        //close  server endpoint
       close(fdServer);
    }

    // read response from server

    return result;
}


int main(int argc, char *argv[])
{
    int     result = EXIT_SUCCESS;
    //double  vDate = 0.0;

    //result = libmessage_getdate("cli_message",SERVER_TIME_ID_GETDATE,&vDate);
//    printf("\ncli_message : result = %d date = %f \n",result,vDate);


    if( argc > 1 )
    {
        result = check_fifo(argv[1]);
    }
    else
    {
        printf("Error no client name ! \n");
    }
    return result;
}
