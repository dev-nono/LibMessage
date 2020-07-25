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
#include <mqueue.h>
#include <poll.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

// include ==>  libmessage.h
#include "utils.h"
#include "libmessage.h"
#include "libmessage_int.h"


int test_msgQueue()
{
    int result = 0;
    int vLenReceive = 0;
    char vClientfilename[PATH_MAX] = "";
    char vServerfilename[PATH_MAX] = SERVER_TIME;
    struct mq_attr  vAttr   = {0};

    struct pollfd fd_client = {0};
    struct pollfd fd_server = {0};

    char    vBufferIN[LIBMESSAGE_MAX_BUFFER] = {0};
    char    vBufferOUT[LIBMESSAGE_MAX_BUFFER] = {0};


    vAttr.mq_flags  = O_CLOEXEC;
    vAttr.mq_curmsgs = 1;
    vAttr.mq_maxmsg = 10;
    vAttr.mq_msgsize = LIBMESSAGE_MAX_BUFFER;

    //**********************************************************
    // open mq server
    //**********************************************************
    fprintf(stderr,"_1_ \n");

    errno = 0;
    fd_server.fd = mq_open(SERVER_TIME,
            O_WRONLY); //,S_IRWXO | S_IRWXG | S_IRWXU ,&vAttr);

    if( -1 == fd_server.fd )
    {
        fprintf(stderr,"mq_open(SERVER_TIME) result=%d errno=%d %s \n",
                result , errno,strerror(errno));
    }
    fprintf(stderr,"_2_ \n");

    if( 0 == result )
    {
        //**********************************************************
        // open mq client
        //**********************************************************
        getMQname(vClientfilename);

        fprintf(stderr,"_3_ \n");

        mq_unlink(vClientfilename);

        errno = 0;
        fd_client.fd = mq_open(vClientfilename,
                O_CREAT |O_RDONLY,S_IRWXO | S_IRWXG | S_IRWXU ,&vAttr);
        if( -1 == fd_client.fd )
        {
            fprintf(stderr,"mq_open(%s) result=%d errno=%d %s \n",
                    vClientfilename,result , errno,strerror(errno));
        }
        fprintf(stderr,"_4_ \n");

        // prepare msg

        do{
//            fprintf(stderr,"_5_ \n");
            // send msg to server
            //int mq_send(mqd_t mqdes, const char *msg_ptr,size_t msg_len, unsigned int msg_prio);

            result = mq_send(fd_server.fd, vClientfilename,
                    strlen(vClientfilename)+1,0);

            result = mq_send(fd_server.fd, vClientfilename,
                    strlen(vClientfilename)+1,0);

            printf("type any key to continue. \n");
            getchar();
            //            fprintf(stderr,"_6_ \n");

            if( 0 != result)
            {
                fprintf(stderr,"mq_send(%d) result=%d errno=%d %s \n",
                        fd_server.fd,result , errno,strerror(errno));
            }

            memset(vBufferOUT,0,sizeof(vBufferOUT));

            //receive msg in client
            vLenReceive =  mq_receive(fd_client.fd,
                    vBufferOUT,
                    LIBMESSAGE_MAX_BUFFER,
                    0U);

            double *pDbl = (double*)&vBufferOUT;
            printf("vLenReceive=%d data= %f\n",
                    vLenReceive,
                    *pDbl);

            sleep(1);
        }while(1);

    }


}

int main(void)
{
    int     result = 0;
    double  vDate = 0.0;

//    result = libmessage_getdate("cli_message",SERVER_TIME_ID_GETDATE,&vDate);
//    printf("\ncli_message : result = %d date = %f \n",result,vDate);

    test_msgQueue();

    return EXIT_SUCCESS;
}
