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
#include <semaphore.h>
#include <signal.h>

#include <sys/types.h>          /* Consultez NOTES */
#include <sys/socket.h>
#include <sys/un.h>

#include "utilstools.h"
#include "utilstools_listtailqueue.h"
#include "utilstools_pthreadutils.h"


#define SRV_GETDATE "/tmp/srv.getdate"
#define CLI_GETDATE "/tmp/cli.getdate"

int sock_cli_read   = 0;
int sock_cli_write  = 0;



typedef struct sRequest
{
    char clientname[NAME_MAX];

}sRequest_t;

typedef struct sResponse
{
    struct timespec date;

}sResponse_t;


#define LISTEN_BACKLOG 50

#define handle_error(msg) \
        do { perror(msg); exit(EXIT_FAILURE); } while (0)



void show_signals(const sigset_t exmask)
{

    int exsignals[43];

    exsignals[0] = SIGABRT;
    exsignals[1] = SIGALRM;
    exsignals[2] = SIGBUS;
    exsignals[3] = SIGCHLD;
    exsignals[4] = SIGCONT;
#ifdef SIGEMT
    exsignals[5] = SIGEMT;
#else
    exsignals[5] = -1;
#endif

    exsignals[6] = SIGFPE;

#ifdef SIGFREEZE
    exsignals[7] = SIGFREEZE;
#else
    exsignals[7] = -1;
#endif

    exsignals[8] = SIGHUP;
    exsignals[9] = SIGILL;
#ifdef SIGINFO
    exsignals[10] = SIGINFO;
#else
    exsignals[10] = -1;
#endif

    exsignals[11] = SIGINT;
    exsignals[12] = SIGIO;
    exsignals[13] = SIGIOT;

#ifdef SIGJVM1
    exsignals[14] = SIGJVM1;
#else
    exsignals[14] = -1;
#endif
#ifdef SIGJVM2
    exsignals[15] = SIGJVM2;
#else
    exsignals[15] = -1;
#endif

    exsignals[16] = SIGKILL;
#ifdef SIGLOST
    exsignals[17] = SIGLOST;
#else
    exsignals[17] = -1;
#endif

#ifdef SIGLWP
    exsignals[18] = SIGLWP;
#else
    exsignals[18] = -1;
#endif

    exsignals[19] = SIGPIPE;
    exsignals[20] = SIGPOLL;
    exsignals[21] = SIGPROF;
    exsignals[22] = SIGPWR;
    exsignals[23] = SIGQUIT;
    exsignals[24] = SIGSEGV;
    exsignals[25] = SIGSTKFLT;
    exsignals[26] = SIGSTOP;
    exsignals[27] = SIGSYS;
    exsignals[28] = SIGTERM;
#ifdef SIGTHAW
    exsignals[29] = SIGTHAW;
#else
    exsignals[29] = -1;
#endif
#ifdef SIGTHR
    exsignals[30] = SIGTHR;
#else
    exsignals[30] = -1;
#endif
    exsignals[31] = SIGTRAP;
    exsignals[32] = SIGTSTP;
    exsignals[33] = SIGTTIN;
    exsignals[34] = SIGTTOU;
    exsignals[35] = SIGURG;
    exsignals[36] = SIGUSR1;
    exsignals[37] = SIGUSR2;
    exsignals[38] = SIGVTALRM;
#ifdef SIGWAITING
    exsignals[39] = SIGWAITING;
#else
    exsignals[39] = -1;
#endif

    exsignals[40] = SIGWINCH;
    exsignals[41] = SIGXCPU;
    exsignals[42] = SIGXFSZ;
#ifdef SIGXRES
    exsignals[43] = SIGXRES;
#else
    exsignals[43] = -1;
#endif

    int exsignals_n = 0;

    for (;exsignals_n < 43; exsignals_n++) {
        if (exsignals[exsignals_n] == -1) continue;
        static char *exsignal_name;
        exsignal_name = strsignal(exsignals[exsignals_n]);
        switch(sigismember(&exmask, exsignals[exsignals_n]))
        {
        case 0: break;
        case 1: printf("YES %s\n", exsignal_name); break;
        case -1: printf("could not obtain signal\n"); break;
        default: printf("UNEXPECTED for %s return\n", exsignal_name); break;
        }
    }
}
const sigset_t getmask(void)
{
    static sigset_t retmask;
    if ((sigprocmask(SIG_SETMASK, NULL, &retmask)) == -1)
        printf("could not obtain process signal mask\n");

    return retmask;
}
int openConnect(const char *a_socketFilename,int* a_pSocketdescriptor)
{
    int result              = 0;
    int Socketdescriptor    = 0;
    int yes                 = 1;

    struct sockaddr_un vSockaddr = {0};

    errno=0;
    //Socketdescriptor = socket(AF_UNIX, SOCK_STREAM, 0);
    Socketdescriptor = socket(AF_UNIX, SOCK_DGRAM, 0);

    if( -1 == Socketdescriptor )
    {
        fprintf(stderr,"%s : socket(AF_UNIX, SOCK_DGRAM)=%d errno=%d %s \n",
                __FUNCTION__,
                Socketdescriptor,errno,strerror(errno) );
        result = errno;
    }

    if( 0 == result )
    {
        result = setsockopt(Socketdescriptor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if( 0 != result )
        {
            fprintf(stderr,"%s : setsockopt(Socketdescriptor, SOL_SOCKET, SO_REUSEADDR,%d)=%d errno=%d %s \n",
                    __FUNCTION__,yes,
                    result,errno,strerror(errno) );
            result = errno;
            close(Socketdescriptor);
        }
    }

    if( 0 == result )
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
            close(Socketdescriptor);
            result = errno;
        }
        else
        {
            *a_pSocketdescriptor = Socketdescriptor;
            result = 0;
        }
    }

    return result;
}
int openBind(const char *a_socketFilename,int* a_pSocketdescriptor)
{
    int result = 0;
    int Socketdescriptor = 0;
    int yes = 1;
    struct sockaddr_un sockaddr = {0};

    errno=0;
    //    Socketdescriptor = socket(AF_UNIX, SOCK_STREAM, 0);
    Socketdescriptor = socket(AF_UNIX, SOCK_DGRAM, 0);

    if( -1 == Socketdescriptor )
    {
        fprintf(stderr,"%s : socket()=%d errno=%d %s \n",
                __FUNCTION__,
                Socketdescriptor,errno,strerror(errno) );
        result = errno;
    }

    if( 0 == result )
    {
        result = setsockopt(Socketdescriptor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if( 0 != result )
        {
            fprintf(stderr,"%s : setsockopt()=%d errno=%d %s \n",
                    __FUNCTION__,
                    result,errno,strerror(errno) );
            result = errno;
            close(Socketdescriptor);
        }
    }

    if( 0 == result)
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
            close(Socketdescriptor);
            result = errno;
        }
        else
        {
            *a_pSocketdescriptor = Socketdescriptor;
        }
    }

    return result;
}
int openBindConnect(const char  *a_clientFilename,
        const char  *a_serverFilename,
        int         *a_pSocketdescriptor)
{
    int result = 0;
    int Socketdescriptor = 0;
    int yes = 1;
    struct sockaddr_un sockaddr = {0};

    if( (! (*a_clientFilename)) || !(*a_serverFilename) || !a_pSocketdescriptor)
    {
        result = EINVAL;
    }

    if( 0 == result)
    {
        Socketdescriptor = socket(AF_UNIX, SOCK_DGRAM, 0);

        if( -1 == Socketdescriptor )
        {
            fprintf(stderr,"%s : socket()=%d errno=%d %s \n",
                    __FUNCTION__,Socketdescriptor,errno,strerror(errno) );
            result = errno;
        }
    }
    if( 0 == result )
    {
        result = setsockopt(Socketdescriptor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if( 0 != result )
        {
            fprintf(stderr,"%s : setsockopt()=%d errno=%d %s \n",
                    __FUNCTION__,result,errno,strerror(errno) );
            result = errno;
            close(Socketdescriptor);
        }
    }

    if( 0 == result)
    {
        unlink(a_clientFilename);

        memset(&sockaddr, 0, sizeof(struct sockaddr_un));

        /* Efface la structure */
        sockaddr.sun_family = AF_UNIX;

        strncpy(sockaddr.sun_path, a_clientFilename,
                sizeof(sockaddr.sun_path) - 1);

        errno = 0;
        result = bind(Socketdescriptor, (struct sockaddr *) &sockaddr,
                sizeof(struct sockaddr_un)) ;

        if( -1 == result  )
        {
            fprintf(stderr,"%s : bind(%d)=%d errn=%d %s \n",
                    __FUNCTION__,Socketdescriptor,
                    result,errno,strerror(errno) );
            close(Socketdescriptor);
            result = errno;
        }
        else
        {
            *a_pSocketdescriptor = Socketdescriptor;
        }
    }

    if( 0 == result)
    {
        memset(&sockaddr, 0, sizeof(struct sockaddr_un));

        /* Efface la structure */
        sockaddr.sun_family = AF_UNIX;

        strncpy(sockaddr.sun_path, a_serverFilename,
                sizeof(sockaddr.sun_path) - 1);

        result = connect(Socketdescriptor, (struct sockaddr *) &sockaddr,
                sizeof(struct sockaddr_un)) ;

        if( -1 == result  )
        {
            fprintf(stderr,"%s : connect(%d,%s)=%d errn=%d %s \n",
                    __FUNCTION__,Socketdescriptor,
                    a_serverFilename,result,errno,strerror(errno) );
            close(Socketdescriptor);
            result = errno;
        }
    }

    return result;
}
typedef int (*libmsg_pFunctCB_t)(const sRequest_t  *a_pRequest,sResponse_t *a_pResponse);

typedef struct sThread
{
    pthread_t   pthreadID;
    char        cliname[NAME_MAX];
    libmsg_pFunctCB_t pFuction;

}sThread_t;

ListQ_t g_listClient =  {0};

typedef struct
{
    int fd;
}sData_t;

pcond_t g_Condition = {0};

static void* threadfunct_srv_job_2(void* a_pArg)
{
    //    int result = 0;

    //    sThread_t *pContext = (sThread_t*) a_pArg;
    //
    //    sRequest_t  request  = {0};
    //    sResponse_t response = {0};
    //
    //    struct timespec ts_date= {0};

    //    int sock_srv_write = -1;
    //    int sock_srv_read  = -1;
    //
    //    printf("%s : _IN_1  \n",__FUNCTION__ );
    //
    //    result = unlink(SRV_GETDATE);
    //
    //    printf("%s : _2_ unlink(%s) = %d  errno=%d %s\n",
    //            __FUNCTION__,SRV_GETDATE,
    //            result,errno,strerror(errno) );
    //
    //    //    result = openBind(SRV_GETDATE,&sock_srv_read);
    //    result = openBind(SRV_GETDATE,&sock_srv_read);
    //
    //    printf("%s : _3_ openBind(%s) = %d sock_srv_read=%d errno=%d %s\n",
    //            __FUNCTION__,SRV_GETDATE,
    //            result,sock_srv_read,errno,strerror(errno) );
    //
    //    do
    //    {
    //        sock_srv_write = -1;
    //        sock_srv_read  = -1;
    //
    //
    //        if( 0 == result )
    //        {
    //            //*******************************************************
    //            // read input
    //            //
    //            result = read(sock_srv_read,&request,sizeof(sRequest_t));
    //
    //            printf("%s _4_ : read(%d,%s) result=%d \n",
    //                    __FUNCTION__ ,sock_srv_read,request.clientname,result);
    //        }
    //        if( 0 == result )
    //        {
    //            //*******************************************************
    //            // connect to input socket
    //            result = openConnect(request.clientname,&sock_srv_write);
    //
    //            printf("%s _5_ : openConnect(%s,%d) result=%d \n",
    //                    __FUNCTION__,
    //                    request.clientname,sock_srv_write,result);
    //
    //        }
    //        if( 0 == result )
    //        {
    //        }
    //        if( 0 == result )
    //        {
    //        }
    //        if( 0 == result )
    //        {
    //        }
    //        if( 0 == result )
    //        {
    //        }
    //        if( 0 == result )
    //        {
    //        }
    //        if( 0 == result )
    //        {
    //        }
    //
    //
    //    }while(1);

    return 0;
}

static void* threadfunct_srv_job(void* a_pArg)
{
    int result = 0;


    sThread_t *pContext = (sThread_t *)a_pArg;

    //    ListQ_item_t *pItem = 0;
    //    sData_t      *pData = 0;
    //    int           sizeList    = 0;

    //    char bufferWrite[PATH_MAX];
    //    char bufferRead[PATH_MAX];
    //    int     sizeBuffer = 0;
    sRequest_t  request  = {0};
    sResponse_t response = {0};



    int sock_srv_read  = -1;
    struct sockaddr_un sockaddr = {0};

    printf("%s : _IN_1  \n",__FUNCTION__ );

    result = unlink(SRV_GETDATE);

    printf("%s : _2_ unlink(%s) = %d  errno=%d %s\n",
            __FUNCTION__,SRV_GETDATE,
            result,errno,strerror(errno) );

    result = openBind(SRV_GETDATE,&sock_srv_read);

    //if( 0 != result)
    {
        printf("%s : _3_ openBind(%s) = %d sock_srv_read=%d errno=%d %s\n",
                __FUNCTION__,SRV_GETDATE,
                result,sock_srv_read,errno,strerror(errno) );

    }//result = openBind(SRV_GETDATE,&sock_srv_read);

    //    if( 0 == result)
    //    {
    //        close(sock_srv_read);
    //        result = openConnect(SRV_GETDATE,&sock_srv_read);
    //        if( 0 != result)
    //        {
    //            printf("%s : _32_ openBind(%s) = %d sock_srv_read=%d errno=%d %s\n",
    //                    __FUNCTION__,SRV_GETDATE,
    //                    result,sock_srv_read,errno,strerror(errno) );
    //        }
    //    }

    if( 0 == result)
    {
        do
        {

            if( 0 == result)
            {
                memset(&sockaddr, 0, sizeof(struct sockaddr_un));

                /* Efface la structure */
                sockaddr.sun_family = AF_UNSPEC;

                result = connect(sock_srv_read, (struct sockaddr *) &sockaddr,
                        sizeof(struct sockaddr_un)) ;

                if( -1 == result  )
                {
                    fprintf(stderr,"%s : connect(%d,%s)=%d errn=%d %s \n",
                            __FUNCTION__,
                            sock_srv_read,
                            request.clientname,
                            result,errno,strerror(errno) );
                    result = errno;
                }
            }


            {
                memset(&request,0,sizeof(sRequest_t));
                memset(&response,0,sizeof(sResponse_t));

                //*******************************************************
                // read input
                //
                //            result = read(p1Data->fd,&request,sizeof(sRequest_t));
                result = read(sock_srv_read,&request,sizeof(sRequest_t));

                printf("%s _4_ : read(%d,%s) result=%d \n",
                        __FUNCTION__ ,sock_srv_read,
                        request.clientname,result);

                //*******************************************************
                // connect to input socket
                //                result = openConnect(request.clientname,&sock_srv_write);
                //                result = openBind(request.clientname,&sock_srv_write);

                //                printf("%s _5_ : openConnect(%s,%d) result=%d \n",
                //                        __FUNCTION__,
                //                        request.clientname,sock_srv_write,result);

                if( -1 == result )
                {
                    // error
                }
                else
                {
                    result = pContext->pFuction(&request,&response);
                }

                if( 0 == result)
                {
                    memset(&sockaddr, 0, sizeof(struct sockaddr_un));

                    /* Efface la structure */
                    sockaddr.sun_family = AF_UNIX;

                    strncpy(sockaddr.sun_path, request.clientname,
                            sizeof(sockaddr.sun_path) - 1);

                    result = connect(sock_srv_read, (struct sockaddr *) &sockaddr,
                            sizeof(struct sockaddr_un)) ;

                    if( -1 == result  )
                    {
                        fprintf(stderr,"%s : connect(%d,%s)=%d errn=%d %s \n",
                                __FUNCTION__,
                                sock_srv_read,
                                request.clientname,
                                result,errno,strerror(errno) );
                        //close(sock_srv_read);
                        result = errno;
                    }
                }
                if( 0 == result )
                {

                    //*******************************************************
                    // write data to ouput socket
                    result = write(sock_srv_read,&response,sizeof(sResponse_t));

                    printf("%s _6_ : write(%d,%ld.%ld) result=%d \n",
                            __FUNCTION__,sock_srv_read,
                            response.date.tv_sec,response.date.tv_nsec,
                            result);

                    result = 0;

                    //close(sock_srv_write);
                }
            }
            //            close(sock_srv_write);

            printf("%s _7_ :  end loop \n",__FUNCTION__);


            //        pcond_unlock(&g_Condition);
            //
            //        sizeList = tq_size(&g_listClient);
            //        printf("%s _8_ : sizeList=%d \n",__FUNCTION__ ,sizeList);

        }while(1);

        close(sock_srv_read);

    }// // 0 = openBind(SRV_GETDATE,&sock_srv_read);


    return 0;
}


static void* threadfunct_srv_bind(void* a_pArg)
{
    int result = 0;

    int sock_srv_read   = 0;
    int sock_srv_write  = 0;

    //    struct sockaddr_un client_address = {0};
    //    unsigned int       client_address_len = sizeof(client_address);;
    //
    //    char buffer[PATH_MAX];
    //    char bufferWrite[PATH_MAX];
    //    char bufferRead[PATH_MAX];
    //    int sizeBuffer = 0;
    //    struct timespec *pDate = (struct timespec *)bufferWrite;

    ListQ_item_t    *pItem      = 0;
    sData_t         *pData      = 0;
    int             sizeList    = 0;

    //        show_signals(getmask());
    //        exit(0);



    printf("%s : IN_1 ",__FUNCTION__);

    errno=0;
    result = unlink(SRV_GETDATE);

    printf("%s : _2_ unlink(%s) = %d  errno=%d %s\n",
            __FUNCTION__,SRV_GETDATE,
            result,errno,strerror(errno) );

    //    result = openBind(SRV_GETDATE,&sock_srv_read);
    result = openConnect(SRV_GETDATE,&sock_srv_read);

    printf("%s : _3_ openBind(%s) = %d sock_srv_read=%d errno=%d %s\n",
            __FUNCTION__,SRV_GETDATE,
            result,sock_srv_read,errno,strerror(errno) );


    tq_init(&g_listClient,sizeof(sData_t));

    //    result = listen(sock_srv_read, 5);
    //
    //    printf("%s : _4_ listen(%d,5) = %d  errno=%d %s \n",
    //            __FUNCTION__,sock_srv_read,
    //            result,errno,strerror(errno) );

    do
    {
        printf("%s : _5_ DO { \n",__FUNCTION__);

        if( 0 != result)
        {
            fprintf(stderr,"%s : listen(%d,5)=%d errno=%d %s \n",
                    __FUNCTION__,  sock_srv_read,
                    result,errno,strerror(errno) );
        }
        else
        {
            //            client_address_len = sizeof(client_address);
            //
            //            sock_srv_write = accept(sock_srv_read,
            //                    (struct sockaddr *)&client_address, &client_address_len);
            //
            //            if( 0 > sock_srv_write)
            //            {
            //                fprintf(stderr,"%s : accept(%d)=%d errno=%d %s \n",
            //                        __FUNCTION__, sock_srv_read,
            //                        sock_srv_write,errno,strerror(errno) );
            //            }
            //            else
            {
                printf("%s : _7_ accept(%d)=%d errno=%d %s\n",__FUNCTION__,
                        sock_srv_read,sock_srv_write,
                        errno,strerror(errno) );

                pcond_lock(&g_Condition);

                printf("%s : _8_ pthread_mutex_lock()\n",__FUNCTION__ );

                sizeList = tq_size(&g_listClient);

                printf("%s : _9_  tq_size(&g_listClient)=%d\n",
                        __FUNCTION__, sizeList);


                tq_lock(&g_listClient);

                // insert
                pItem = tq_insertTail(&g_listClient);


                pData = (sData_t*)pItem->pData;

                pData->fd = sock_srv_write ;

                tq_unlock(&g_listClient);

                pcond_signal(&g_Condition);


                sizeList = tq_size(&g_listClient);
                printf("%s : _10_  tq_size(&g_listClient)=%d\n",
                        __FUNCTION__, sizeList);

                pcond_unlock(&g_Condition);

                sizeList = tq_size(&g_listClient);

                printf("%s : _11_  tq_size(&g_listClient)=%d\n",
                        __FUNCTION__, sizeList);
            }// if( 0 > sock_srv_write)
        } //do  if( 0 != result)


        if( 0 == result)
        {
            // notify thread to make job
        }

    }while(1);

    //    if( 0 == result )
    //    {
    //        do
    //        {
    //            memset(bufferRead,0,PATH_MAX);
    //
    //            result = read(sock_srv_read,bufferRead,PATH_MAX-1);
    //
    //            printf("%s : read =%d  %s \n",
    //                    __FUNCTION__,result, buffer);
    //
    //            result = openConnect(bufferRead,&sock_srv_write);
    //            //result = open(bufferRead,0);
    //
    //            if( 0 != result )
    //            {
    //                sock_srv_write = result;
    //
    //                result = clock_gettime(CLOCK_MONOTONIC_RAW,pDate);
    //
    //                sizeBuffer = sizeof(struct timespec);;
    //
    //                printf("%s : date=%ld.%09ld \n",
    //                        __FUNCTION__,
    //                        pDate->tv_sec,
    //                        pDate->tv_nsec);
    //
    //                result = write(sock_srv_write,bufferWrite,sizeBuffer);
    //
    //                close(sock_srv_write);
    //            }
    //
    //        }while(1);
    //    }
    return (void*)0;

}
char * getClientName(const char * a_strID ,char* a_cli_srvname)
{
    snprintf(a_cli_srvname,NAME_MAX-50,"%s_%s",CLI_GETDATE,a_strID);

    return a_cli_srvname;
}


int clientReadData(const sRequest_t  *a_pRequest,sResponse_t *a_pResponse)
{
    int result      = 0;
    int sizeBuffer  = 0;
    int sock_client = -1;

    printf("%s_1 : \n",__FUNCTION__ );

    if(( !a_pRequest) || (!a_pResponse))
    {
        result = EINVAL;
    }

    if( 0 == result)
    {
        unlink(a_pRequest->clientname);

        result = openBindConnect(a_pRequest->clientname,SRV_GETDATE,&sock_client);

        printf("%s_3 : openBindConnect(%s,%d) result=%d \n",
                __FUNCTION__ ,a_pRequest->clientname,sock_client,result);
    }

    //    if( 0 == result)
    //    {
    //        close(sock_client);
    //
    //        result = openConnect(a_pRequest->clientname,&sock_client);
    //
    //        printf("%s_32 : openBind(%s,%d) result=%d \n",
    //                __FUNCTION__ ,a_pRequest->clientname,sock_client,result);
    //
    //    }

    if( 0 == result)
    {
        sizeBuffer = strlen(a_pRequest->clientname) + 1;
        //        result = write(sock_server,a_pRequest,sizeBuffer);
        result = write(sock_client,a_pRequest,sizeBuffer);

        printf("%s_4 : write(%d,%d) result=%d errno=%d %s\n",
                __FUNCTION__ ,sock_client,sizeBuffer,result,
                errno , strerror(errno));
        if( result >= 0)
            result = 0;
        else
            result = -1;
    }

    if( 0 == result)
    {
        //poll
        result = read(sock_client,a_pResponse,sizeof(sResponse_t));

        printf("%s_7 : read(%d,%lu) result=%d \n",
                __FUNCTION__ ,sock_client,sizeof(sResponse_t),result);

        if( -1 == result )
            result = errno;
        else
            result = 0;
    }

    close(sock_client);
    //    close(sock_server);

    return result;
}
static int startClient(int argc, char *argv[])
{
    int result = 0;

    sRequest_t  request     = {0};
    sResponse_t response    = {0};

    getClientName(argv[2],request.clientname);

    do{
        printf("%s_1 : type any key to continue \n",__FUNCTION__);

        getchar();

        memset(&response,0,sizeof(sResponse_t ));

        result = clientReadData(&request,&response);
        if ( 0 == result )
        {
            printf("%s : clientReadData()=%d  date=%ld.%ld \n",
                    __FUNCTION__,result,
                    response.date.tv_sec,
                    response.date.tv_nsec);
        }
        else
        {
            printf("%s : error clientReadData()=%d %s \n",
                    __FUNCTION__,result,strerror(result));

        }


    }while(1);

    return result;
}
//*******************************************
//* tst cli 1
//*******************************************
static int startClient1(int argc, char *argv[])
{
    int result = 0;
    //    int curchar = 0;

    //    char buffer[PATH_MAX];
    //    char pBuffer = 0;

    //char cli_srvname[PATH_MAX] = {0};

    //    char bufferWrite[PATH_MAX];
    //    char bufferRead[PATH_MAX];
    int sizeBuffer = 0;


    // int sock_client_bind = -1;
    int sock_client = -1;
    int sock_server = -1;



    /* sock_srv = openconnect(srvname)
     *
     * sock_client = openbind(cli_name)
     *
     *  write(sock_srv,request)
     *  read(sock_client,response)
     *
     * close(sock_srv)
     * close(sock_client)
     */

    sRequest_t  request     = {0};
    sResponse_t response    = {0};
    //    struct sockaddr_un client_address = {0};
    //    unsigned int       client_address_len = sizeof(client_address);;

    getClientName(argv[2],request.clientname);


    do{
        printf("%s_1 : type any key to continue \n",__FUNCTION__);

        getchar();

        sock_client  = -1;
        sock_server = -1;

        //        result = openBind(SRV_GETDATE,&sock_server);
        result = openConnect(SRV_GETDATE,&sock_server);

        printf("%s_2 : openBind(%s,%d) result=%d \n",
                __FUNCTION__ ,SRV_GETDATE,sock_server,result);

        unlink(request.clientname);
        result = openBind(request.clientname,&sock_client);

        printf("%s_3 : openBind(%s,%d) result=%d \n",
                __FUNCTION__ ,SRV_GETDATE,sock_client,result);

        //        result = listen(sock_client_bind, 1);
        //        printf("%s_4 : listen(%d,1) result=%d errno=%d %s\n",
        //                __FUNCTION__ ,sock_client_bind,result,
        //                errno , strerror(errno));
        //
        //        sock_client = accept(sock_client_bind,
        //                (struct sockaddr *)&client_address, &client_address_len);
        //
        //        printf("%s_5 : accept(%d) result=%d errno=%d %s\n",
        //                __FUNCTION__ ,sock_client_bind,
        //                result,errno , strerror(errno));

        sizeBuffer = strlen(request.clientname) + 1;
        result = write(sock_server,&request,sizeBuffer);

        printf("%s_6 : write(%d,%d) result=%d errno=%d %s\n",
                __FUNCTION__ ,sock_server,sizeBuffer,result,
                errno , strerror(errno));

        result = read(sock_client,&response,sizeof(sResponse_t));

        printf("%s_7 : read(%d,%lu) result=%d \n",
                __FUNCTION__ ,sock_client,sizeof(sResponse_t),result);

        close(sock_client);
        close(sock_server);

        printf("read_%s : %ld.%ld \n",
                argv[2],
                response.date.tv_sec,
                response.date.tv_nsec);

    }while(1);


    return result;
}


int funcCB_srv_getdate(const sRequest_t  *a_pRequest,sResponse_t *a_pResponse)
{
    int result = 0;

    result = clock_gettime(CLOCK_MONOTONIC_RAW,&a_pResponse->date);

    printf("%s : clock_gettime()=%d  date=%ld.%ld \n",
            __FUNCTION__,result,a_pResponse->date.tv_sec,a_pResponse->date.tv_nsec);

    return result;
}

int start_Server(char * a_srvname)
{
    int result = 0;



    //sThread_t srv_bind    = {0};
    sThread_t srv_job     = {0};

    strncpy(srv_job.cliname,a_srvname,NAME_MAX-1);
    srv_job.pFuction = funcCB_srv_getdate;

    //    result =  pthread_create(&srv_bind.pthreadID,
    //            NULL,
    //            &threadfunct_srv_bind,
    //            (void*)&srv_bind);
    //
    //    sleep(1);


    result =  pthread_create(&srv_job.pthreadID,
            NULL,
            &threadfunct_srv_job,
            (void*)&srv_job);


    //    pthread_join(srv_bind.pthreadID,0);

    pthread_join(srv_job.pthreadID,0);

    return result;
}

static void print_usage(char* argv0)
{
    fprintf(stderr,"\n syntaxe error : %s [cmd] [ID] \n",argv0);
    fprintf(stderr," [cmd] : cli ID\n");
    fprintf(stderr," [cmd] : srv \n");
    fprintf(stderr," [ID] : 1, 2 ,3. ... \n\n");

}
int main_socket(int argc, char *argv[])
{
    int result = 0;

    pcond_init(&g_Condition);

    if( argc > 1 )
    {
        // SERVER
        if( 0 == strcmp(argv[1],"srv")  )
        {
            result = start_Server(SRV_GETDATE);
        }
        else if( 0 == strcmp(argv[1],"cli")  )
        {
            result = startClient(argc, argv);
        }
        else
        {
            print_usage(argv[0]);
            result = -1;
        }
    }
    else
    {
        print_usage(argv[0]);
        result = -1;
    }

    return result;
}
