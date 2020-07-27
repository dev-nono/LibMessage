/*
 ============================================================================
 Name        : server_time.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <limits.h>
#include <mqueue.h>
#include <time.h>
#include <poll.h>
#include <errno.h>

#include "apisyslog.h"
#include "utils.h"


#include "libmsg.h"
//#include "libmessage_int.h"
//#include "libmessage.h"

int test_msgQueue()
{
#define LIBMESSAGE_MAX_BUFFER 1024

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
 //   mq_unlink(SERVER_TIME);
    errno  = 0;
    fd_server.fd = mq_open(SERVER_TIME,
            O_CREAT | O_RDONLY,S_IRWXO | S_IRWXG | S_IRWXU ,&vAttr);

    fprintf(stderr,"_1_ \n");

//    printf("type any key to continue. \n");
//    getchar();

    if( -1 == fd_server.fd )
    {
        fprintf(stderr,"mq_open(SERVER_TIME) result=%d errno=%d %s \n",
                result , errno,strerror(errno));
    }
    // prepare msg

    do{
//        fprintf(stderr,"_2_ \n");

        fd_server.events = POLLIN | POLLPRI;
        fd_server.revents = 0;

        errno = 0;
        result = poll(&fd_server,1,-1);
        if ( (0 == result) || (-1 == result))
        {
            fprintf(stderr,"poll(%d) result=%d errno=%d %s \n",
                    fd_server.fd,result , errno,strerror(errno));
        }
//        fprintf(stderr,"_3_ \n");

        if ( 0 < result)
        {
//            fprintf(stderr,"_4_ \n");
            memset(vClientfilename,0,sizeof(vClientfilename));

            errno = 0;
            result = mq_receive(fd_server.fd, vClientfilename,
                    sizeof(vClientfilename),0);
//            fprintf(stderr,"_5_ \n");

            if ( (0 == result) || (-1 == result))
            {
                fprintf(stderr,"poll(%d) result=%d errno=%d %s \n",
                        fd_server.fd,result , errno,strerror(errno));
            }
            else
            {
                result = 0;
            }

//            printf("vClientfilename=%s \n",vClientfilename);
        }
//        fprintf(stderr,"_6_ \n");

        if( 0 == result )
        {
//            fprintf(stderr,"_7_ \n");
            //**********************************************************
            // open mq client
            //**********************************************************
            errno = 0;
            fd_client.fd = mq_open(vClientfilename,
                    O_WRONLY);//,S_IRWXO | S_IRWXG | S_IRWXU ,&vAttr);

            if( -1 == fd_server.fd )
            {
                fprintf(stderr,"mq_open(vClientfilename) result=%d errno=%d %s \n",
                        result , errno,strerror(errno));
                result = errno;
            }
        }


//        fprintf(stderr,"_8_ \n");

        if( 0 == result )
        {
            memset(vBufferOUT,0,sizeof(vBufferOUT));

//            fprintf(stderr,"_9_ \n");

            double dblValue = getDateRawDouble();
            memcpy(vBufferOUT,&dblValue,sizeof(dblValue));
            // send msg to server
            //int mq_send(mqd_t mqdes, const char *msg_ptr,size_t msg_len, unsigned int msg_prio);

            errno = 0;
            result = mq_send(fd_client.fd, vBufferOUT,
                    sizeof(dblValue),0);

            fprintf(stderr,"_10_ %s=%f result=%d sizeof(dblValue)=%lu errno=%d %s\n",
                    vClientfilename,dblValue,result,sizeof(dblValue),
                    errno,strerror(errno));

            if ( 0 != result)
            {
                fprintf(stderr,"mq_send(%d) result=%d errno=%d %s \n",
                        fd_client.fd,result , errno,strerror(errno));
            }
        }
//        fprintf(stderr,"_11_ \n");

        mq_close(fd_client.fd);

        //        //receive msg in client
        //        vLenReceive =  mq_receive(fd_server.fd,
        //                vBufferOUT,
        //                LIBMESSAGE_MAX_BUFFER,
        //                0U);

        //sleep(1);
    }while(1);
}
static int libmsg_cbfcnt_getdate(void *a_pContext)
{
    int result = 0;
    char msgbuffer[APISYSLOG_MSG_SIZE] = {0};

    //sRequest_t *  pRequest   = (sRequest_t*) a_pRequest;

    // no inputdata in request
    sRequest_t *  pRequest   =     &( (sDataThreadCtx_t*) a_pContext)->dataService.request;
    sResponse_t * pResponse  =     &( (sDataThreadCtx_t*) a_pContext)->dataService.response;

//    sGetdateResponse_t  *pData      = (sGetdateResponse_t *)&pResponse->data;
//
//    pResponse->header.datasize =
//            sizeof(sHeader_t) + sizeof(sGetdateResponse_t);
//
//    result = clock_gettime(CLOCK_MONOTONIC_RAW,
//            &pData->timespesc);
//
//    snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
//            " : date=%ld.%09ld result=%d\n",
//            pData->timespesc.tv_sec,
//            pData->timespesc.tv_nsec,
//            result);
//
//    pResponse->header.result = result;

    TRACE_LOG(msgbuffer);

    return result;
}
//static int libmessage_cbfcnt_setdate(void* a_pData)
//{
//    int result = 0;
//
//    return result;
//}
//static int libmessage_cbfcnt_signal(void* a_pData)
//{
//    int result = 0;
//
//    return result;
//}

//*********************************************************
//*
//*********************************************************
int main(void)
{
    int result = 0;

    test_msgQueue();

    //    result =  libmessage_register_service(
    //            LIBMESSAGE_SRVID_TIME,
    //            SERVER_TIME_ID_GETDATE ,
    //            &libmessage_cbfcnt_getdate);

    //    result = libmessage_register_service_time( SERVER_TIME_SETDATE, libmessage_cbfcnt_setdate);
    //    result = libmessage_register_service_time( SERVER_TIME_SIGNAL,  libmessage_cbfcnt_signal,     libmessage_cbfcnt_signal);

    result = libmsg_srvtime_register_getdate(libmsg_cbfcnt_getdate);
    result = libmsg_srvtime_wait();

    return result;
}

