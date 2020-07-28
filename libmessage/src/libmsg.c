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
#include <pthread.h>

#include "apisyslog.h"
#include "utils.h"

#include "libmsg_int.h"
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

    errno = 0;
    fd_server.fd = mq_open(a_Srvname,O_WRONLY);
                //,S_IRWXO | S_IRWXG | S_IRWXU ,&vAttr);

    if( -1 == fd_server.fd )
    {
        fprintf(stderr,"mq_open(%s) result=%d errno=%d %s \n",
                a_Srvname,result , errno,strerror(errno));
    }

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
    }
    if( 0 == result )
    {
        fushMessageQueue( fd_client.fd);
    }
    // prepare msg

    if( 0 == result )
    {

        // send msg to server
        result = mq_send(fd_server.fd, a_BufferIN,a_SizeBuffIn,0);

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
    }

    mq_close(fd_client.fd);
    mq_close(fd_server.fd);

    return result;
}

static void * libmsg_srv_threadFunction(void * a_pArg)
{
    int             result          = 0;

    sDataThreadCtx_t *pContex = (sDataThreadCtx_t *)a_pArg;

    struct pollfd   fd_client   = {0};
    struct pollfd   fd_server   = {0};
    struct mq_attr  vAttr       = {0};
    char        msgbuffer[APISYSLOG_MSG_SIZE]   = {0};

    char    vBufferIN[HARD_MAX]     = {0};
    uint32_t SizeBuffIn             = 0;
    char    vBufferOUT[HARD_MAX]    = {0};
    uint32_t SizeBuffOut            = sizeof(vBufferOUT);


    mq_unlink(pContex->dataService.filenameServer);

    vAttr.mq_flags  = O_CLOEXEC;
    vAttr.mq_curmsgs = 1;
    vAttr.mq_maxmsg = 10;
    vAttr.mq_msgsize = HARD_MAX;


    //**********************************************************
    // open mq server
    //**********************************************************
    errno  = 0;
    fd_server.fd = mq_open(pContex->dataService.filenameServer,
            O_CREAT | O_RDONLY,S_IRWXO | S_IRWXG | S_IRWXU ,&vAttr);

    fprintf(stderr,"_1_ \n");

    if( -1 == fd_server.fd )
    {
        result = errno;
        snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                "mq_open(%s) result=%d errno=%d %s \n",
                pContex->dataService.filenameServer,
                result , errno,strerror(errno));
        fprintf(stderr,"%s : %s \n",__FUNCTION__, msgbuffer);
        TRACE_ERR(msgbuffer);
    }

TRACE_DBG1("_1_")
    if(0 == result)
    {
        do{
            TRACE_DBG1("_2_")
                        //***********************************************************
            //                  POLL
            //***********************************************************
            fd_server.events = POLLIN | POLLPRI;
            fd_server.revents = 0;

            errno = 0;
            result = poll(&fd_server,1,-1);
            TRACE_DBG1("_3_ poll=0x%X",result);

            if ( 0 == result) // timeout
            {
                snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                        "error poll(%d) result=%d TIMEOUT \n",
                        fd_server.fd,
                        result );
                TRACE_ERR(msgbuffer);
                result = -1;
                continue;
            }
            else if ( -1 == result )
            {
                snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                        "error poll(%d) result=%d errno=%d %s \n",
                        fd_server.fd,
                        result , errno,strerror(errno));
                TRACE_ERR(msgbuffer);
            }
            else
            {
                result = 0; // to chech event
            }

            //***********************************************************
            //                  RECEIVE DATA
            //***********************************************************
            if ( 0 == result)
            {
                TRACE_DBG1("_4_");
               //            fprintf(stderr,"_4_ \n");
                memset(vBufferOUT,0,sizeof(vBufferOUT));

                errno = 0;
                result = mq_receive(fd_server.fd, vBufferOUT,
                        sizeof(vBufferOUT),0);
                //            fprintf(stderr,"_5_ \n");
                TRACE_DBG1("_5_ result=0x%X",result);

                if ( (0 == result) || (-1 == result))
                {
                    snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                            "error mq_receive(%d) result=%d errno=%d %s \n",
                            fd_server.fd,
                            result , errno,strerror(errno));
                    TRACE_ERR(msgbuffer);
                    continue;
                }
                else
                {
                    result = 0;
                }

                //            printf("vClientfilename=%s \n",vClientfilename);
            }
            //***********************************************************
            //                open client
            //***********************************************************
            if ( 0 == result)
            {
                TRACE_DBG1("_6_ result=0x%X",result);
             errno = 0;
                fd_client.fd = mq_open(vBufferOUT,O_WRONLY);

                if( -1 == fd_client.fd )
                {
                    result = errno;
                    snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                            "mq_open(%.50s) result=%d errno=%d %s \n",
                            vBufferOUT,
                            result , errno,strerror(errno));
                    TRACE_ERR(msgbuffer);
                }
            }
            if( 0 == result )
            {
                TRACE_DBG1("_7_ result=0x%X",result);
               memset(vBufferIN,0,sizeof(vBufferIN));

    //            fprintf(stderr,"_9_ \n");

                double dblValue = getDateRawDouble();
                memcpy(vBufferIN,&dblValue,sizeof(dblValue));
                // send msg to server
                errno = 0;
                result = mq_send(fd_client.fd, vBufferIN,
                        sizeof(dblValue),0);

                TRACE_DBG2("_8_ %s=%f result=%d errno=%d %s\n",
                        vBufferOUT,dblValue,result,errno,strerror(errno));

                if ( 0 != result)
                {
                    snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                            "error mq_send(%.50s) result=%d errno=%d %s \n",
                            vBufferOUT,
                            result , errno,strerror(errno));
                    TRACE_ERR(msgbuffer);
                }
            }
    //        fprintf(stderr,"_11_ \n");

            mq_close(fd_client.fd);

        }while(1);

    }// if(0 == result)

    return (void*)0;
}

//************************************************************
//*
//************************************************************
int libmsg_srv_register_svc(sDataThreadCtx_t *a_pDataThreadCtx)
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
                &libmsg_srv_threadFunction,
                (void*)a_pDataThreadCtx);

        if( 0 != result )
        {
            snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                    ": pthread_create() error =%d %s",
                    result,strerror(result));

            TRACE_ERR(msgbuffer);
        }
    }

    return result;
}
