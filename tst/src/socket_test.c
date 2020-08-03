/*
 * socket_test.c
 *
 *  Created on: 2 août 2020
 *      Author: bruno
 */



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>          /* Consultez NOTES */
#include <sys/socket.h>
#include <sys/un.h>

#define SRV_GETDATE "/tmp/srv.getdate"
#define CLI_GETDATE "/tmp/cli.getdate"

int sock_cli_read   = 0;
int sock_cli_write  = 0;



#define LISTEN_BACKLOG 50

#define handle_error(msg) \
        do { perror(msg); exit(EXIT_FAILURE); } while (0)


int openConnect(const char *a_socketFilename,int* a_pSocketdescriptor)
{
    int result = 0;
    int Socketdescriptor = 0;

    struct sockaddr_un vSockaddr = {0};

    errno=0;
    Socketdescriptor = socket(AF_UNIX, SOCK_STREAM, 0);

//    errno=0;
//    result = unlink(a_socketFilename);

    if( -1 != Socketdescriptor )
    {
        /* Efface la structure */
        vSockaddr.sun_family = AF_UNIX;

        strncpy(vSockaddr.sun_path, a_socketFilename,
                sizeof(vSockaddr.sun_path) - 1);

        errno = 0;
        result = connect(Socketdescriptor, (struct sockaddr *) &vSockaddr,
                sizeof(struct sockaddr_un)) ;

        if( -1 == result  )
        {
            fprintf(stderr,"%s : connect(%d,%s)=%d errn=%d %s \n",
                    __FUNCTION__,
                    Socketdescriptor,
                    a_socketFilename,
                    result,errno,strerror(errno) );
        }
    }
    else
    {
        *a_pSocketdescriptor = Socketdescriptor;
        fprintf(stderr,"%s : socket()=%d errno=%d %s \n",
                __FUNCTION__,
                result,errno,strerror(errno) );
    }


    return result;
}
int openBind(const char *a_socketFilename,int* a_pSocketdescriptor)
{
    int result = 0;
    int Socketdescriptor = 0;

    struct sockaddr_un sockaddr = {0};

    errno=0;
    Socketdescriptor = socket(AF_UNIX, SOCK_STREAM, 0);

//    errno=0;
//    result = unlink(a_socketFilename);

    if( -1 != Socketdescriptor )
    {
        memset(&sockaddr, 0, sizeof(struct sockaddr_un));

        /* Efface la structure */
        sockaddr.sun_family = AF_UNIX;

        strncpy(sockaddr.sun_path, a_socketFilename,
                sizeof(sockaddr.sun_path) - 1);

        errno = 0;
        result = bind(Socketdescriptor, (struct sockaddr *) &sockaddr,
                sizeof(struct sockaddr_un)) ;

        if( -1 == result  )
        {
            fprintf(stderr,"%s : bind(%d)=%d errn=%d %s \n",
                    __FUNCTION__,
                    Socketdescriptor,
                    result,errno,strerror(errno) );
        }
    }
    else
    {
        *a_pSocketdescriptor = Socketdescriptor;
        fprintf(stderr,"%s : socket()=%d errno=%d %s \n",
                __FUNCTION__,
                result,errno,strerror(errno) );
    }


    return result;
}
static void* threadfunct_start_server(void* a_pArg)
{
    int result = 0;

    int sock_srv_read   = 0;
    int sock_srv_write  = 0;

    char buffer[PATH_MAX];

    char bufferWrite[PATH_MAX];
    char bufferRead[PATH_MAX];
    int sizeBuffer = 0;
    struct timespec *pDate = (struct timespec *)bufferWrite;

        errno=0;
        result = unlink(SRV_GETDATE);

    result = openBind(SRV_GETDATE,&sock_srv_read);

    if( 0 == result )
    {
        do
        {
            memset(bufferRead,0,PATH_MAX);

            result = read(sock_srv_read,bufferRead,PATH_MAX-1);

            printf("%s : read =%d  %s \n",
                    __FUNCTION__,result, buffer);

            result = openConnect(bufferRead,&sock_srv_write);
            //result = open(bufferRead,0);

            if( 0 != result )
            {
                sock_srv_write = result;

                result = clock_gettime(CLOCK_MONOTONIC_RAW,pDate);

                sizeBuffer = sizeof(struct timespec);;

                printf("%s : date=%ld.%09ld \n",
                        __FUNCTION__,
                        pDate->tv_sec,
                        pDate->tv_nsec);

                result = write(sock_srv_write,bufferWrite,sizeBuffer);

                close(sock_srv_write);
            }

        }while(1);
    }
    return (void*)0;

}

static int start_client()
{
    int result = 0;

    result = openConnect(SRV_GETDATE,&sock_cli_write);

    if( -1 != result )
    {
        result = openBind(CLI_GETDATE,&sock_cli_read);

        if( 0 != result )
        {
            close(sock_cli_write);
        }
    }


   return result;
}


int main_socket(int argc, char *argv[])
{
    int result = 0;

    char buffer[PATH_MAX];

    char bufferWrite[PATH_MAX];
    char bufferRead[PATH_MAX];
    int sizeBuffer = 0;

    pthread_t pthreadID = 0;

    result =  pthread_create(&pthreadID,
            NULL,
            &threadfunct_start_server,
            0);

//    threadfunct_start_server(0);

    sleep(1);

    result = start_client();

    do{
        printf("%s : type any key to continue \n",__FUNCTION__);
        fgets(buffer,255-1,stdin);

        strncpy(bufferWrite,CLI_GETDATE,PATH_MAX-1);
        sizeBuffer = strlen(CLI_GETDATE);
        result = write(sock_cli_write,bufferWrite,sizeBuffer);

        memset(bufferRead,0,sizeof(bufferRead));
        result = read(sock_cli_read,bufferRead,PATH_MAX-1);

        struct timespec *pTS = (struct timespec *)bufferRead;

        printf("read =%d  %ld.%ld \n",
                result, pTS->tv_sec,pTS->tv_nsec);


    }while(1);



    return result;
}
