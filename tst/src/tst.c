/*
 ============================================================================
 Name        : tst.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <fcntl.h> /* Définition des constantes AT_* */
#include <unistd.h>
#include <fcntl.h>           /* Pour les constantes O_* */
#include <sys/stat.h>        /* Pour les constantes des modes */
#include <mqueue.h>
#include <string.h>
#include <errno.h>


#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <limits.h>

#define MQ_FILENAME "/server_time"


//#define STRINGIFY(x) #x
//#define STRINGIFY_2(aa,bb) STRINGIFY(aa ## bb)
//#define STRINGIFY_3(aa,bb,cc) STRINGIFY(aa ## bb ## cc)
//
////#define CONCAT2( _a , _b )     _a _b
////#define CONCAT_SEM( _a , _b )  STRINGIFY( _a._b )
//
////#define CONCAT(  _a , _b )     CONCAT3( _a ,_b )
//
//
////#define  SVR_TIME() STRINGIFY(srvtime)
//#define  SVR_TIME           srvtime
//
//#define SVC_GETDATE         getdate
//#define SVC_SETDATE         setdate
//#define SVC_SIGNALDATE      signaldate
//
//#define SVR_TIME_GETDATE_CONCAT(aa,bb)   STRINGIFY_3( aa , . , bb )
//
//#define SVR_TIME_GETDATE                    SVR_TIME_GETDATE_CONCAT( SVR_TIME , SVC_GETDATE )
////#define SVR_TIME_SETDATE                    SVR_TIME_GETDATE_CONCAT( SVR_TIME , SVC_SETDATE )
////
////#define SVR_TIME_GETDATE_CONCAT_SEM(aa , bb)    STRINGIFY3( aa , _ , bb )
////#define SVR_TIME_GETDATE_SEM                   SVR_TIME_GETDATE_CONCAT_SEM( SVR_TIME,SVC_GETDATE )


#define STRINGIFY(x) #x
#define STRINGIFY_2(aa,bb)      STRINGIFY(aa ## bb)
#define STRINGIFY_3(aa,bb,cc)   STRINGIFY(aa ## bb ## cc)

#define CONCAT2(_a,_b) _a _b
#define CONCAT_SEM(_a,_b) STRINGIFY( _a._b )





#define SVR_TIME srvtime

#define SVC_GETDATE getdate
#define SVC_SETDATE setdate
#define SVC_SIGNALDATE signaldate

#define CONCAT_POINT(_a,_b) STRINGIFY( _a._b ) // ok "."
#define CONCAT_U(aa,bb) STRINGIFY_3( aa , _ , bb ) // ok "_"


#define SVR_TIME_GETDATE CONCAT_POINT( SVR_TIME,SVC_GETDATE )
#define SVR_TIME_SETDATE CONCAT_POINT( SVR_TIME,SVC_SETDATE )

#define SVR_TIME_GETDATE_SEM CONCAT_U( SVR_TIME,SVC_GETDATE )





int check_preprocessor()
{

    //printf("%s  \n",SVR_TIME_GETDATE_SEM);
    printf("%s  \n",SVR_TIME_GETDATE);
    printf("%s  \n",SVR_TIME_SETDATE);
    printf("%s  \n",SVR_TIME_GETDATE_SEM);
    return 0;
}

int check_mq(void) {

    int result = 0;
    struct mq_attr  vAttr   = {0};

    vAttr.mq_flags  = O_CLOEXEC;
    vAttr.mq_curmsgs = 9;
    vAttr.mq_maxmsg = 9;
    vAttr.mq_msgsize = 1024;

    errno = 0;
    //result = mq_open(MQ_FILENAME, O_CREAT,S_IRWXG,&vAttr);
    //    result = mq_open(MQ_FILENAME, O_CREAT,S_IRWXG,0); // ok
    result = mq_open(MQ_FILENAME, O_CREAT,S_IRWXO | S_IRWXG | S_IRWXU ,&vAttr); // ok

    printf("mq_open(%s) result=%d errno=%d %s\n",
            MQ_FILENAME,result,errno, strerror(errno));


    return EXIT_SUCCESS;
}

sigset_t    sigset_mask = {0};

void * thread_signal(void * a_pArg)
{
    int result = 0;
    int    vSignal = 0;

    do
    {
        vSignal = 0;
        result = sigwait(&sigset_mask, &vSignal);

        printf("%s : sigwait(mask,%d) result = %d \n",
                __FUNCTION__,vSignal,result);

    }while(1);

    return 0;
}
int check_signal()
{
    pthread_t   tid = 0;
    int         result = 0;

    sigemptyset(&sigset_mask);
    sigaddset(&sigset_mask, SIGUSR1);
    sigaddset(&sigset_mask, SIGUSR2);
    sigaddset(&sigset_mask, SIGALRM);
    sigprocmask(SIG_BLOCK, &sigset_mask, NULL);

    pthread_create(&tid, NULL, thread_signal, NULL);

    printf("%s : type anykey SIGUSR1\n",__FUNCTION__);
    getchar();

    do
    {
        result = pthread_kill(tid,SIGUSR1);
        printf("%s : pthread_kill(tid,SIGUSR1) signal=%d result = %d \n",
                __FUNCTION__,SIGUSR1,result);
        getchar();

        printf("%s : type anykey SIGUSR2\n",__FUNCTION__);
        result = pthread_kill(tid,SIGUSR2);
        printf("%s : pthread_kill(tid,SIGUSR2) signal=%d result = %d \n",
                __FUNCTION__,SIGUSR2,result);
        getchar();

        printf("%s : type anykey SIGALRM\n",__FUNCTION__);
        result = pthread_kill(tid,SIGALRM);
        printf("%s : pthread_kill(tid,SIGALRM)signal=%d result = %d \n",
                __FUNCTION__,SIGALRM,result);
        getchar();

        printf("%s : type anykey SIGUSR1\n",__FUNCTION__);

    }while(1);

}

    //******************************************************
    //
    //******************************************************
int check_mkfifo()
{
    int result = 0;
    char msgbuffer[255] = {0};
    char    endpointName[PATH_MAX] = "/tmp/client.123456";

    unlink(endpointName);

    //*********************************************************
    // create server endpoint
    //*********************************************************
    errno = 0;
    result = mkfifo(endpointName,S_IRWXU);

    if( (0 != result ) && (EEXIST != errno) )
    {
        // error
        snprintf(msgbuffer,255,
                ": mkfifo(-%s-) Error=%d %s \n",
                endpointName,
                errno,strerror(errno));

        fprintf(stderr,"%s : %s \n",__FUNCTION__, msgbuffer);

    }
    else
    {
        fprintf(stderr,": mkfifo(%s) OK \n",
                endpointName);
        result = 0;
    }

    return result;

}
int main(void)
{

    // check_mq();
    //check_preprocessor();
    // check_signal();

    check_mkfifo();

}
