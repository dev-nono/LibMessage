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

#include <stdio.h>
#include <stdlib.h>

#define MQ_FILENAME "/server_time"

#include "utils.h"

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

int main(int argc , char *argv[] )
{

    //tst_mq_open();

    // tst_clock();

    tst_ts_split_double();

}
