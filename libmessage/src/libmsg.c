/*
 * libmsg.c
 *
 *  Created on: 27 juil. 2020
 *      Author: bruno
 */

#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#include <fcntl.h>           /* Pour les constantes O_* */
#include <sys/stat.h>        /* Pour les constantes des modes */
#include <mqueue.h>


#include "apisyslog.h"
#include "utils.h"

#include "libmsg.h"

//#include "libmessage_int.h"
//#include "libmessage.h"


int libmsg_cli_getdata(
        _IN_ const char *a_Srvname,
        _IN_ const char *a_Clientname,
        _IN_ const uint32_t a_SizeBuffIn,
        _IN_ const char *a_BufferIN,
        _IN_ const uint32_t a_SizeBuffOut,
        _OUT_      char *a_BufferOUT)
{
    int result = 0;
    int vLenReceive = 0;
//    char vClientfilename[PATH_MAX] = "";
//    char vServerfilename[PATH_MAX] = SERVER_TIME;
    struct mq_attr  vAttr   = {0};

    struct pollfd fd_client = {0};
    struct pollfd fd_server = {0};

//    char    vBufferIN[LIBMESSAGE_MAX_BUFFER] = {0};
//    char    vBufferOUT[LIBMESSAGE_MAX_BUFFER] = {0};


    vAttr.mq_flags  = O_CLOEXEC;
    vAttr.mq_curmsgs = 1;
    vAttr.mq_maxmsg = 1;
    vAttr.mq_msgsize = HARD_MAX;

    //**********************************************************
    // open mq server
    //**********************************************************
    fprintf(stderr,"_1_ \n");

    errno = 0;
    fd_server.fd = mq_open(a_Srvname,O_WRONLY);
                //,S_IRWXO | S_IRWXG | S_IRWXU ,&vAttr);

    if( -1 == fd_server.fd )
    {
        fprintf(stderr,"mq_open(%s) result=%d errno=%d %s \n",
                a_Srvname,result , errno,strerror(errno));
    }
    fprintf(stderr,"_2_ \n");

    if( 0 == result )
    {
        //**********************************************************
        // open mq client
        //**********************************************************
        mq_unlink(a_Clientname);

        errno = 0;
        fd_client.fd = mq_open(a_Clientname,
                O_CREAT | O_RDONLY , S_IRWXO | S_IRWXG | S_IRWXU ,&vAttr);
        if( -1 == fd_client.fd )
        {
            fprintf(stderr,"mq_open(%s) result=%d errno=%d %s \n",
                    a_Clientname,result , errno,strerror(errno));
        }
        fprintf(stderr,"_4_ \n");
    }
    if( 0 == result )
    {
        fushMessageQueue( fd_client.fd);
    }
    // prepare msg

    if( 0 == result )
    {

        fprintf(stderr,"_5_ \n");
        // send msg to server
        result = mq_send(fd_server.fd, a_BufferIN,a_SizeBuffIn,0);


        fprintf(stderr,"_6_ \n");

        if( 0 != result)
        {
            fprintf(stderr,"mq_send(%d) result=%d errno=%d %s \n",
                    fd_server.fd,result , errno,strerror(errno));
        }
    }

    if( 0 == result )
    {

        memset(a_BufferOUT,0,a_SizeBuffOut);

        //receive msg in client
        vLenReceive =  mq_receive(fd_client.fd,
                a_BufferOUT,
                HARD_MAX,
                0U);
        printf("vLenReceive=%d \n",vLenReceive);
    }

    mq_close(fd_client.fd);
    mq_close(fd_server.fd);

    return result;
}

static void * libmsg_server_threadFunction(void * a_pArg)
{
    int             result          = 0;

    return (void*)(result!=0);
}

//************************************************************
//*
//************************************************************
int libmsg_server_register_svc(sDataThreadCtx_t *a_pDataThreadCtx)
{
    int result = 0;
    char msgbuffer[APISYSLOG_MSG_SIZE] = {0};

    //**************************************************
    //*  create semaphore
    //**************************************************
//    if( 0 == result )
//    {
//        errno = 0;
//        result = sem_unlink(a_pDataThreadCtx->dataService.filenameSemaphore);
//        if( 0 != result )
//        {
//            snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
//                    ": sem_unlink(%s) result=%d errno=%d %s",
//                    a_pDataThreadCtx->dataService.filenameSemaphore,
//                    result,errno,strerror(errno));
//
//            fprintf(stderr,"%s : %s \n",__FUNCTION__, msgbuffer);
//            TRACE_ERR(msgbuffer);
//        }
//
//        result = 0;
//        errno = 0;
//        a_pDataThreadCtx->dataService.pSemsvc = sem_open(
//                a_pDataThreadCtx->dataService.filenameSemaphore,
//                O_CREAT,S_IRWXU,1U);
//        if( SEM_FAILED == a_pDataThreadCtx->dataService.pSemsvc)
//        {
//            snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
//                    "sem_open(%s) result=0x%p errno=%d %s",
//                    a_pDataThreadCtx->dataService.filenameSemaphore,
//                    (void*)a_pDataThreadCtx->dataService.pSemsvc,
//                    errno,strerror(errno));
//
//            fprintf(stderr,"%s : %s\n",__FUNCTION__, msgbuffer);
//            TRACE_ERR(msgbuffer);
//        }
//    }

    if( 0 == result )
    {
       //*****************************
        // create new tread for listening incomming messages
        //*****************************
        errno = 0;
        result =  pthread_create(&a_pDataThreadCtx->pthreadID,
                NULL,
                &libmsg_server_threadFunction,
                (void*)a_pDataThreadCtx);

        if( 0 != result )
        {

            snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                    ": pthread_create() error =%d %s",
                    result,strerror(result));

            fprintf(stderr,"%s : %s \n",__FUNCTION__, msgbuffer);
            TRACE_ERR(msgbuffer);
        }
    }

    return result;
}
