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
        _IN_ const  char        *a_Srvname,
            const   sRequest_t  *a_pRequest,
                    sResponse_t *a_pResponse
        )
{
    int result = 0;
    int vLenReceive = 0;
    struct mq_attr  vAttr   = {0};

    struct pollfd fd_client = {0};
    struct pollfd fd_server = {0};

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
        mq_unlink(a_pRequest->filenameClient);

        errno = 0;
        fd_client.fd = mq_open(a_pRequest->filenameClient,
                O_CREAT | O_RDONLY , S_IRWXO | S_IRWXG | S_IRWXU ,&vAttr);
        if( -1 == fd_client.fd )
        {
            fprintf(stderr,"mq_open(%s) result=%d errno=%d %s \n",
                    a_pRequest->filenameClient,
                    result , errno,strerror(errno));
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
        result = mq_send(fd_server.fd, (char*)a_pRequest,
                sizeof(sRequest_t),0);

        if( 0 != result)
        {
            fprintf(stderr,"mq_send(%d) result=%d errno=%d %s \n",
                    fd_server.fd,result , errno,strerror(errno));
        }
    }

    if( 0 == result )
    {
        memset(a_pResponse,0,sizeof(sResponse_t));

        //receive msg in client
        vLenReceive =  mq_receive(fd_client.fd,
                (char*)a_pResponse,
                sizeof(sResponse_t),
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

    char    buffRequest[1204*100];
    sRequest_t  request = {0};
    sResponse_t response= {0};


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

            memset(&request ,0,sizeof(request));
            memset(&response,0,sizeof(response));

            TRACE_DBG1("_2_");
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
                memset( buffRequest,0,sizeof(buffRequest));

                errno = 0;
                result = mq_receive(fd_server.fd, buffRequest,
                        sizeof(buffRequest),0);
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
                memcpy(&request, buffRequest,sizeof(request));

                errno = 0;
                fd_client.fd = mq_open(request.filenameClient,O_WRONLY);

                if( -1 == fd_client.fd )
                {
                    result = errno;
                    snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                            "mq_open(%.50s) result=%d errno=%d %s \n",
                            request.filenameClient,
                            result , errno,strerror(errno));
                    TRACE_ERR(msgbuffer);
                }
            }
            if( 0 == result )
            {
                TRACE_DBG1("_7_ result=0x%X",result);

                result = pContex->dataService.pFunctCB(&request,&response);

               // send msg to server
                errno = 0;
                result = mq_send(fd_client.fd, (char*)&response,
                        response.header.datasize,0);

                TRACE_DBG2("_8_mq_send()=%d size=%d errno=%u %s\n",
                        result,
                        response.header.datasize,
                        errno,strerror(errno));

                if ( 0 != result)
                {
                    snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                            "error mq_send() result=%d errno=%d %s \n",
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

    return result;
}
