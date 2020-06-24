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


#include <stdio.h>
#include <stdlib.h>

#define MQ_FILENAME "/server_time"

int main(void) {

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
