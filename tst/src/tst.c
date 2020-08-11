/*
 ============================================================================
 Name        : tst.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <fcntl.h>           /* Pour les constantes O_* */
#include <sys/stat.h>        /* Pour les constantes des modes */
#include <mqueue.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <stdint.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/queue.h> // double list

#include <stdio.h>
#include <stdlib.h>

#define MQ_FILENAME "/server_time"

#include <utilstools.h>
#include <utilstools_listtailqueue.h>
#include "utilstools_time.h"

int main_socket(int argc, char *argv[]);



int   tst_clock()
{
    int result = 0;

    struct timeval  time_v = {0,1e6 / 1000 * 10};
    struct timeval  time_v_result = {0};

    struct timespec ts_timeout = {0,1e9 / 1000 *10 };
    struct timespec ts_result = {0};

    // timeradd_gtofd(time_v,&time_v_result);


    struct timespec ts_curtime = {0,0};

    result = clock_gettime(CLOCK_REALTIME, &ts_curtime);

    timeradd_ts(ts_curtime,ts_timeout,&ts_result);

    return result;
}

int  tst_ts_split_double()
{
    int     result  = 0;
    double  date    = 123456789.123456789;
    struct timespec  ts ,ts1,ts2, ts3, ts4,ts5 = {0};
    double d1,d2 = 0.0;

    unsigned int udate = (unsigned int)date;
    double      fdate = (double)udate;

    uint64_t    u_sec = 0;
    uint64_t    u_nsec =0;
    uint64_t    u_diff =0;


    date    = 12345678.123456789;

    u_sec = (uint64_t)date * 1e9;
    u_nsec = date * 1e9;
    u_diff = u_nsec - u_sec ;

    ts.tv_sec = udate;
    ts.tv_nsec = (date - fdate)  * 1e9;


    date    = 12345678.123456789;
    udate = (unsigned int)date;
    fdate = (double)udate;
    ts1.tv_sec = udate;
    ts1.tv_nsec = (date - fdate)  * 1e9;


    date    = 1234567.123456789;
    udate = (unsigned int)date;
    fdate = (double)udate;
    ts2.tv_sec = udate;
    ts2.tv_nsec = (date - fdate)  * 1e9;

    date    = 123456.123456789;
    udate = (unsigned int)date;
    fdate = (double)udate;
    ts3.tv_sec = udate;
    ts3.tv_nsec = (date - fdate)  * 1e9;

    date    = 12345.123456789;
    udate = (unsigned int)date;
    fdate = (double)udate;
    ts4.tv_sec = udate;
    ts4.tv_nsec = (date - fdate)  * 1e9;

    date    = 1234.123456789;
    udate = (unsigned int)date;
    fdate = (double)udate;
    ts5.tv_sec = udate;
    ts5.tv_nsec = (date - fdate)  * 1e9;


    date = 123456789.112233445;
    u_sec = (uint64_t)date * 1e9;
    u_nsec = date * 1e9;
    u_diff = u_nsec - u_sec ;

    date = 12345678.112233445;
    u_sec = (uint64_t)date * 1e9;
    u_nsec = date * 1e9;
    u_diff = u_nsec - u_sec ;

    date = 1234567.112233445;
    u_sec = (uint64_t)date * 1e9;
    u_nsec = date * 1e9;
    u_diff = u_nsec - u_sec ;

    date = 123456.112233445;
    u_sec = (uint64_t)date * 1e9;
    u_nsec = date * 1e9;
    u_diff = u_nsec - u_sec ;

    date = 12345.112233445;
    u_sec = (uint64_t)date * 1e9;
    u_nsec = date * 1e9;

    date = 1234.112233445;
    u_sec = (uint64_t)date * 1e9;
    u_nsec = date * 1e9;
    u_diff = u_nsec - u_sec ;



    date    = 123456789.112233445;
    d1 = 0.0;
    d2 = modf(date,&d1);

    date    = 12345678.123456789;
    d1 = 0.0;
    d2 = modf(date,&d1);

    date    = 1234567.123456789;
    d1 = 0.0;
    d2 = modf(date,&d1);

    date    = 123456.123456789;
    d1 = 0.0;
    d2 = modf(date,&d1);

    date    = 12345.123456789;
    d1 = 0.0;
    d2 = modf(date,&d1);

    date    = 1234.123456789;
    d1 = 0.0;
    d2 = modf(date,&d1);

    date    = 9999.123456789;
    d1 = 0.0;
    d2 = modf(date,&d1);


    //d3 =
    //    d2 = remainder(date,1);
    //
    //
    //    d1 = fmod(date,(double)ts.tv_sec);
    //    d2 = remainder(date,(double)ts.tv_sec);

    return result;
}

int tst_mq_open()
{

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

static void handler(int sig, siginfo_t *si, void *unused)
{
    printf("handler : signal Reçu %d  \n",sig);

    //signal(sig,SIG_IGN);

}

static void *threadFunction_signal(void* a_pArg)
{
    int result = 0;
    int signalreceive = 0;

    struct sigaction    sSigaction = {0};
    sigset_t            sigset      = {0};
    siginfo_t       siginfo     = {0};

    //*************************************************
    //*************************************************
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGUSR1);
    sigaddset(&sigset, SIGUSR2);
    sigaddset(&sigset, SIGALRM);

    result = pthread_sigmask(SIG_BLOCK, &sigset, NULL);

    //*************************************************
    //*************************************************
    //    sSigaction.sa_flags = SA_SIGINFO;
    //    sigemptyset(&sSigaction.sa_mask);
    //
    //    sigaddset(&sSigaction.sa_mask, SIGUSR1);
    ////    sigaddset(&sSigset.sa_mask, SIGUSR2);
    ////    sigaddset(&sSigset.sa_mask, SIGALRM);
    //
    //    //sSigset.sa_sigaction = handler;
    //
    //    result = sigaction(SIGUSR1, &sSigaction, NULL);
    //    result = sigaction(SIGUSR2, &sSigaction, NULL);
    //    result = sigaction(SIGALRM, &sSigaction, NULL);

    printf("threadFunction_signal_1 \n");
    do{

        signalreceive = 0;
        memset(&siginfo,0,sizeof(siginfo));

        printf("threadFunction_signal_2 \n");

        //        pause(); // ok sigaction + handler
        //        result = sigwait(&sigset, &signalreceive); ok pthread_sigmask
        //        result = sigwait(&sigset, &signalreceive);
        //        printf("threadFunction_signal_3 result = %d  signalreceive=%d \n",
        //        result,signalreceive);

        result = sigwaitinfo(&sigset, &siginfo);
        printf("threadFunction_signal_3 result = %d  signalreceive=%d \n",
                result,siginfo.si_signo);

    }while(1);

    return (void*)0;
}

int tst_signal()
{
    int result = EXIT_SUCCESS;
    int cmd = 0;

    pthread_t pthreadID = 0;


    // create thread
    result =  pthread_create(&pthreadID,
            NULL,
            &threadFunction_signal,
            (void*)0);


    do{
        printf("type a key to continue : 1 , 2 , a=alarm \n");
        cmd = getchar();

        if( '1' == cmd )
        {
            // send SIGUSR1
            pthread_kill(pthreadID,SIGUSR1);
        }
        else if( '2' == cmd )
        {
            pthread_kill(pthreadID,SIGUSR2);
            // send SIGUSR2
        }
        else if( 'a' == cmd )
        {
            // send SIGALRM
            pthread_kill(pthreadID,SIGALRM);

        }
        else
        {

        }




    }while(1);



    return result;

}


#define SETITEM( XXX )    pDate ##XXX = (sData_t*)pItem ##XXX->pData; pDate ##XXX->ID   = XXX;


struct sData
{
    int ID;
};
typedef struct sData sData_t;



int printTQ(ListQ_t *a_pListQ)
{
    ListQ_item_t *pItem = 0;

    printf(">>> ");

    FOR_TQ(pItem,a_pListQ)
    {
        sData_t *pData = (sData_t*)pItem->pData;
        printf("%d ",pData->ID);
        //printf("%p ",pItem->pData);
    }

    printf(" <<<\n");

    return 0;
}
ListQ_t g_ListID;
ListQ_item_t *pItem  = 0;
ListQ_item_t *pItem1 = 0;
ListQ_item_t *pItem2 = 0;
ListQ_item_t *pItem3 = 0;
ListQ_item_t *pItem4 = 0;
ListQ_item_t *pItem5 = 0;
ListQ_item_t *pItem6 = 0;
ListQ_item_t *pItem7 = 0;
ListQ_item_t *pItem8 = 0;
ListQ_item_t *pItem9 = 0;
ListQ_item_t *pItem10 = 0;

sData_t *pDate1 = 0;
sData_t *pDate2 = 0;
sData_t *pDate3 = 0;
sData_t *pDate4 = 0;
sData_t *pDate5 = 0;
sData_t *pDate6 = 0;
sData_t *pDate7 = 0;
sData_t *pDate8 = 0;
sData_t *pDate9 = 0;

void test_list_insert(int a_nbitem)
{

    if(  a_nbitem >= 2 )
    {
        pItem2 = tq_insertTail(&g_ListID);    SETITEM(2);
        //pDate2 = pItem2->pData;pDate2->ID   = 2;        pItem2->pData = (void*)2;
    }
    if(  a_nbitem >= 3 )
    {
        pItem3 = tq_insertAfter(&g_ListID,pItem2);SETITEM(3);
        //pDate3 = pItem3->pData;pDate3->ID   = 3;        pItem3->pData = (void*)3;
    }
    if(  a_nbitem >= 4 )
    {
        pItem4 = tq_insertTail(&g_ListID);SETITEM(4);
        //pDate4 = pItem4->pData;pDate4->ID   = 4;        pItem4->pData = (void*)4;
    }

    if(  a_nbitem >= 5 )
    {
        pItem5 = tq_insertTail(&g_ListID);SETITEM(5);
        //pDate5 = pItem5->pData;pDate5->ID   = 5;        pItem5->pData = (void*)5;
    }
    if(  a_nbitem >= 1 )
    {
        pItem1 = tq_insertHead(&g_ListID);SETITEM(1);
        //pDate1 = pItem1->pData;pDate1->ID   = 1;        pItem1->pData = (void*)1;
    }

    if(  a_nbitem >= 6 )
    {
        pItem6 = tq_insertTail(&g_ListID);SETITEM(6);
    }
    if(  a_nbitem >= 7 )
    {
        pItem7 = tq_insertTail(&g_ListID);SETITEM(7);
    }
    if(  a_nbitem >= 8 )
    {
        pItem8 = tq_insertTail(&g_ListID);SETITEM(8);
    }

    printTQ(&g_ListID);
}
int test_list()
{
    tq_init(&g_ListID,sizeof(sData_t));

    //****************************************************
    test_list_insert(5);

    pItem6 = tq_createItemList(&g_ListID);    SETITEM(6);
//    pDate6 = pItem6->pData;pDate6->ID   = 6;
//    pItem6->pData = (void*)6;

    tq_insertItemHead(&g_ListID,pItem6);

    printf("\n\t*** 1 tq_tq_insertItemHead(6) \n");
    printTQ(&g_ListID);
//**************************************
    pItem7 = tq_createItemList(&g_ListID); SETITEM(7);
//    pDate7 = pItem7->pData;pDate7->ID   = 7;
//    pItem7->pData = (void*)7;


    tq_insertItemTail(&g_ListID,pItem7);

    printf("\n\t*** 2 tq_insertItemTail(7) \n");
    printTQ(&g_ListID);

//**************************************

    pItem8 = tq_createItemList(&g_ListID);  SETITEM(8);
//    pDate8 = pItem8->pData;pDate8->ID   = 8;
//    pItem8->pData = (void*)8;

    tq_insertItemAfter(&g_ListID,pItem3,pItem8);

    printf("\n\t*** 3 tq_insertItemAfter(8,3) \n");
    printTQ(&g_ListID);

    //****************************************************
    tq_removeItemList(&g_ListID,pItem8);
    printf("\n\t*** 4  tq_removeItemList(8) \n");
    printTQ(&g_ListID);

    //****************************************************
    tq_removeTailList(&g_ListID);
    printf("\n\t*** 5  tq_removeTailList() \n");
    printTQ(&g_ListID);

    //****************************************************
    tq_removeHeadList(&g_ListID);
    printf("\n\t*** 6  tq_removeHeadList() \n");
    printTQ(&g_ListID);


    tq_clearList(&g_ListID);
    printf("\n\t*** 7  tq_clearList\n");
    printTQ(&g_ListID);



    test_list_insert(8);

    tq_eraseItemList(&g_ListID,pItem6);
    tq_eraseItemList(&g_ListID,pItem7);
    tq_eraseItemList(&g_ListID,pItem8);

    printf("\n\t*** 8  tq_eraseItemList( 6 7 8)\n");
    printTQ(&g_ListID);

    tq_eraseList(&g_ListID);
    printf("\n\t*** 8  tq_eraseList()\n");
    printTQ(&g_ListID);


    return 0;
}
/*
 * DoStuff
 */
void DoStuff(void) {

 printf("Timer went off.\n");
 raise(SIGALRM);
}
int tst_itimer()
{
    int result = 0;
    struct itimerval iVal = {0};
    struct timespec date1 = {0};
    struct timespec date2 = {0};

    char buff[10] = {0};

    struct sigaction    sSigaction = {0};
    sigset_t            sigset      = {0};
    siginfo_t       siginfo     = {0};

    //*************************************************
    //*************************************************
//    sigemptyset(&sigset);
//    sigaddset(&sigset, SIGALRM);
//     sigaddset(&sigset, SIGINT);
//   result = pthread_sigmask(SIG_BLOCK, &sigset, NULL);
//


    iVal.it_value.tv_sec  = 1;
    iVal.it_value.tv_usec = 0;

//    iVal.it_interval = iVal.it_value;
//    iVal.it_interval.tv_sec  = 1;
//    iVal.it_interval.tv_usec = 0;

    signal(SIGALRM, (void (*)(int)) DoStuff);

    clock_gettime(CLOCK_MONOTONIC_RAW,&date1);

    result = setitimer(ITIMER_REAL, &iVal,0);

    //pause();
    result = read(0,buff,1);

    clock_gettime(CLOCK_MONOTONIC_RAW,&date2);

    printf("\n %ld.%ld\n %ld.%ld\n result=%d\n",
            date1.tv_sec,date1.tv_nsec,
            date2.tv_sec,date2.tv_nsec,
            result);

    return 0;
}
int main(int argc , char *argv[] )
{

//    tst_mq_open();
//
//    tst_clock();
//
//    tst_ts_split_double();
//
//    tst_signal();
//
    main_socket(argc, argv);
//
//        tst_itimer();
//
//    test_list();
}
