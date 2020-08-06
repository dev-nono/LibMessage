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

#include "listtailqueue.h"

#define SRV_GETDATE "/tmp/srv.getdate"
#define CLI_GETDATE "/tmp/cli.getdate"

int sock_cli_read   = 0;
int sock_cli_write  = 0;



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
        else
        {
            *a_pSocketdescriptor = Socketdescriptor;
            result = 0;
        }
    }
    else
    {
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
        else
        {
            *a_pSocketdescriptor = Socketdescriptor;
        }
    }
    else
    {
        fprintf(stderr,"%s : socket()=%d errno=%d %s \n",
                __FUNCTION__,
                result,errno,strerror(errno) );
    }


    return result;
}

ListQ_t g_listClient =  {0};

typedef struct
{
    int fd;
}sData_t;

pthread_cond_t g_Condition          = PTHREAD_COND_INITIALIZER;
pthread_mutex_t g_MutexCondition    = PTHREAD_MUTEX_INITIALIZER;


static void* threadfunct_srv_job(void* a_pArg)
{
    int result = 0;

    ListQ_item_t *pItem = 0;
    sData_t      *pData = 0;
    int             sizeList    = 0;

//    show_signals(getmask());
//    exit(0);

    printf("%s : _IN_1  \n",__FUNCTION__ );

    do
    {
        printf("%s _2_ : DO \n",__FUNCTION__ );

        pthread_mutex_lock(&g_MutexCondition);

        printf("%s _3_ : DO \n",__FUNCTION__ );

        result = pthread_cond_wait(&g_Condition,&g_MutexCondition);

        printf("%s : _4_ pthread_cond_wait()=%d  errno=%d %s \n",
                __FUNCTION__,result,errno,strerror(errno) );

        if( 0 != result )
        {
            // error
        }
        else
        {
            sizeList = tq_size(&g_listClient);
            printf("%s _5_ : sizeList=%d \n",__FUNCTION__ ,sizeList);

            tq_lock(&g_listClient);

            pItem = tq_removeHeadList(&g_listClient);

            pData = (sData_t*)tq_destroyItem(pItem,0);

            tq_unlock(&g_listClient);

            printf("%s _6_ : pData->fd=%d \n",__FUNCTION__ ,pData->fd);

           close(pData->fd);

            free(pData);

            sizeList = tq_size(&g_listClient);
            printf("%s _7_ : sizeList=%d \n",__FUNCTION__ ,sizeList);
        }


        pthread_mutex_unlock(&g_MutexCondition);

        sizeList = tq_size(&g_listClient);
        printf("%s _8_ : sizeList=%d \n",__FUNCTION__ ,sizeList);

    }while(1);


    return 0;
}
static void* threadfunct_srv_bind(void* a_pArg)
{
    int result = 0;

    int sock_srv_read   = 0;
    int sock_srv_write  = 0;

    struct sockaddr_un client_address = {0};
    unsigned int       client_address_len = sizeof(client_address);;

    char buffer[PATH_MAX];
    char bufferWrite[PATH_MAX];
    char bufferRead[PATH_MAX];
    int sizeBuffer = 0;
    struct timespec *pDate = (struct timespec *)bufferWrite;

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

    result = openBind(SRV_GETDATE,&sock_srv_read);

    printf("%s : _3_ openBind(%s) = %d sock_srv_read=%d errno=%d %s\n",
            __FUNCTION__,SRV_GETDATE,
            result,sock_srv_read,errno,strerror(errno) );


    tq_init(&g_listClient,sizeof(sData_t));

    result = listen(sock_srv_read, 5);

    printf("%s : _4_ listen(%d,5) = %d  errno=%d %s \n",
            __FUNCTION__,sock_srv_read,
            result,errno,strerror(errno) );

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
            client_address_len = sizeof(client_address);

            sock_srv_write = accept(sock_srv_read,
                    (struct sockaddr *)&client_address, &client_address_len);

            if( 0 > sock_srv_write)
            {
                fprintf(stderr,"%s : accept(%d)=%d errno=%d %s \n",
                        __FUNCTION__, sock_srv_read,
                        sock_srv_write,errno,strerror(errno) );
            }
            else
            {
                printf("%s : _7_ accept(%d)=%d errno=%d %s\n",__FUNCTION__,
                        sock_srv_read,sock_srv_write,
                        errno,strerror(errno) );

                if( 0 != result )
                {

                }
                else
                {

                    pthread_mutex_lock(&g_MutexCondition);

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

                    result = pthread_cond_signal(&g_Condition);

                    sizeList = tq_size(&g_listClient);
                    printf("%s : _10_  tq_size(&g_listClient)=%d\n",
                            __FUNCTION__, sizeList);
                }

                pthread_mutex_unlock(&g_MutexCondition);

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

static int start_client()
{
    int result = 0;

//    result = openConnect(SRV_GETDATE,&sock_cli_write);
//
//    if( -1 != result )
//    {
//        result = openBind(CLI_GETDATE,&sock_cli_read);
//
//        if( 0 != result )
//        {
//            close(sock_cli_write);
//        }
//    }


    return result;
}



int main_socket(int argc, char *argv[])
{
    int result = 0;

    char buffer[PATH_MAX];
    char pBuffer = 0;

    char bufferWrite[PATH_MAX];
    char bufferRead[PATH_MAX];
    int sizeBuffer = 0;

    pthread_t pthreadID_srv_bind    = 0;
    pthread_t pthreadID_srv_job     = 0;
    pthread_t pthreadID_client      = 0;


    pthread_mutexattr_t attrib;

    pthread_mutexattr_init(&attrib);
    pthread_mutexattr_settype(&attrib, PTHREAD_MUTEX_DEFAULT);
    pthread_mutex_init(& g_MutexCondition,&attrib);
    pthread_mutexattr_destroy(&attrib);


//    show_signals(getmask());
//    exit(0);

    result =  pthread_create(&pthreadID_srv_bind,
            NULL,
            &threadfunct_srv_bind,
            0);

    //    threadfunct_start_server(0);

    sleep(1);

    result =  pthread_create(&pthreadID_srv_job,
            NULL,
            &threadfunct_srv_job,
            0);

    sleep(1);

//    result = start_client();

    int curchar = 0;

    do{
        printf("%s : type any key to continue \n",__FUNCTION__);
        //fgets(buffer,2,stdin);
//        do{
//            //curchar = getchar();
//            errno = 0;
//            pBuffer = fgets(buffer,2,stdin);
//
//        }while(curchar != '\n');

        //errno = 0;
        //pBuffer = fgets(buffer,2,stdin);

        curchar = getchar();
//        read(0,buffer,1);


        sock_cli_write = 0;

        result = openConnect(SRV_GETDATE,&sock_cli_write);

        printf("%s : openConnect(%s,%d)=%d \n",
                __FUNCTION__,SRV_GETDATE,sock_cli_write, result);

    close(sock_cli_write);

//        strncpy(bufferWrite,CLI_GETDATE,PATH_MAX-1);
//        sizeBuffer = strlen(CLI_GETDATE);
//        result = write(sock_cli_write,bufferWrite,sizeBuffer);
//
//        memset(bufferRead,0,sizeof(bufferRead));
//        result = read(sock_cli_read,bufferRead,PATH_MAX-1);
//
//        struct timespec *pTS = (struct timespec *)bufferRead;
//
//        printf("read =%d  %ld.%ld \n",
//                result, pTS->tv_sec,pTS->tv_nsec);


    }while(1);



    return result;
}
