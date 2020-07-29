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

    tst_clock();

}
