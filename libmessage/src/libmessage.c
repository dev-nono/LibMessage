/*
 * libmessage.c
 *
 *  Created on: 5 juin 2020
 *      Author: bruno
 */


#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>

#include <sys/stat.h>
#include <unistd.h>
#include <math.h>

#include "apisyslog.h"
#include <utils.h>

#include "libmessage_svc_time.h"
#include "libmessage_int.h"



static sDataThreadCtx_t g_arrayTheadCtx[eLIBMSG_ID_END]= {0};

static char  g_arrayNameService[eLIBMSG_ID_END][3][NAME_MAX]=
{
        {SVC_TIME_GETDATE_PREFIX,      SVR_TIME_GETDATE_SEM,    SRV_TIME_GETDATE_FILENAME},
        {SVC_TIME_SETDATE_PREFIX,      SVR_TIME_SETDATE_SEM,    SRV_TIME_SETDATE_FILENAME},
        {SVC_TIME_SIGNALDATE_PREFIX,   SVR_TIME_SIGNALDATE_SEM, SRV_TIME_SIGNALDATE_FILENAME}

};

sDataThreadCtx_t *getTheadCtx(eLIBMSG_ID_t a_ID)
{
    return &g_arrayTheadCtx[a_ID];
}

const char *getNameService(eLIBMSG_ID_t a_ID,eLIBMSG_COL_t a_ColID)
{
    return g_arrayNameService[a_ID][a_ColID];

}
//************************************************************
//*
//************************************************************
int libmessage_srvtime_register_svc(    eLIBMSG_ID_t            a_MessageID,
                                        libmessage_pFunctCB_t   a_pFunctCB )
{
    int result = 0;
    sDataThreadCtx_t *pDataThreadCtx = 0;
    char msgbuffer[APISYSLOG_MSG_SIZE] = {0};

    if( (a_MessageID < 0) || (a_MessageID >=  eLIBMSG_ID_END) )
    {
        result = EINVAL;
    }

    //*****************************
    // prepare data thread
    //*****************************
    if( 0 == result )
    {
        pDataThreadCtx = getTheadCtx(a_MessageID);
    }

    //*****************************
    // prepare data thread
    //*****************************
    if( 0 == result )
    {
        pDataThreadCtx->dataService.pFunctCB = a_pFunctCB;

        strncpy(pDataThreadCtx->dataService.filenameServer,
                getNameService(a_MessageID,eLIBMSG_COL_SRV_FILENAME),
                sizeof(pDataThreadCtx->dataService.filenameServer)-1);
    }

    //**************************************************
    //*  create semaphore
    //**************************************************
    if( 0 == result )
    {
        errno = 0;
        result = sem_unlink(getNameService(a_MessageID,eLIBMSG_COL_SEM));
        if( 0 != result )
        {
            snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                    ": sem_unlink(%s) result=%d errno=%d %s",
                    getNameService(a_MessageID,eLIBMSG_COL_SEM),
                    result,errno,strerror(errno));

            fprintf(stderr,"%s : %s \n",__FUNCTION__, msgbuffer);
            TRACE_ERR(msgbuffer);
        }

        result = 0;
        errno = 0;
        pDataThreadCtx->dataService.pSemsvc = sem_open(
                getNameService(a_MessageID,eLIBMSG_COL_SEM),
                O_CREAT,S_IRWXU,1U);
        if( SEM_FAILED == pDataThreadCtx->dataService.pSemsvc)
        {
            snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                    "sem_open(%s) result=0x%p errno=%d %s",
                    getNameService(a_MessageID,eLIBMSG_COL_SEM),
                    (void*)pDataThreadCtx->dataService.pSemsvc,
                    errno,strerror(errno));

            fprintf(stderr,"%s : %s\n",__FUNCTION__, msgbuffer);
            TRACE_ERR(msgbuffer);
        }
    }

    if( 0 == result )
    {
       //*****************************
        // create new tread for listening incomming messages
        //*****************************
        errno = 0;
        result =  pthread_create(&pDataThreadCtx->pthreadID,
                NULL,
                &libmessage_threadFunction_srv,
                (void*)pDataThreadCtx);

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

//************************************************************
//* generique function called by client to get data
//  input data :
//      .filenameClient
//      .filenameServer
//      .pSemsvc
//  output data :
//      .databuffer
//
//  return: > 0 OK len data read
//          error number
//************************************************************
int libmessage_svc_getdata(sDataService_t *a_pDataService)
{
    int     result = 0;
    int     fdServer = -1;
    int     vSize = 0;
    char    msgbuffer[APISYSLOG_MSG_SIZE] = {0};

    struct pollfd   vPollfdClient = {0};
    nfds_t          vNfds    = 1;
    int             vTimeout = 500;


    vPollfdClient.fd      = -1;

    //*********************************************************
    //          create client fifo
    //*********************************************************
    vSize =  NAME_MAX -1 - strlen("/tmp/client.");
     snprintf(a_pDataService->request.filenameClient,
             vSize,
             "/tmp/client.%s",
             a_pDataService->filenameClientSuffix);

    result = libmessage_mkfifo(a_pDataService->request.filenameClient);

    //*********************************************************
    //          open client endpoint
    //*********************************************************
    if( 0 == result )
    {
        result = libmessage_openfifo(a_pDataService->request.filenameClient,O_RDONLY,&vPollfdClient.fd );
    }

    //*********************************************************
    //          open server fifo
    //*********************************************************
    if( 0 == result )
    {
        // open server endpoint  argv[1]
        fdServer = -1;
        result = libmessage_openfifo(a_pDataService->filenameServer,O_WRONLY,&fdServer);
    }

    if( 0 == result )
    {
        struct timespec vdatedelta = {0,1e9/1000*100}; // 100ms
        struct timespec abs_timeout = {0,0};

        ADD_TIMESPEC_REAL(vdatedelta,abs_timeout);

        //***************************************************
        //              lock
        //***************************************************
        result = sem_timedwait(a_pDataService->pSemsvc,&abs_timeout);
        if( 0 != result )
        {
            snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                    " : sem_wait() result=%d err=%d %s",
                    result,errno,strerror(errno));

            fprintf(stderr,"%s : %s \n",__FUNCTION__, msgbuffer);
            TRACE_ERR(msgbuffer);
        }
    }

    //*********************************************************
    //          write request
    //*********************************************************
    if( 0 == result )
    {
        //send request to server endpoint
        vSize = sizeof(a_pDataService->request);
        errno = 0;
        result = write(fdServer,
                (char*)&a_pDataService->request,
                 vSize);

        if(-1 ==  result)
        {
            result = errno;
            snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                    " : _2_ server write(%d,%s,%d) Error %d %s ",
                    fdServer,
                    a_pDataService->filenameServer,
                    vSize,errno,strerror(errno));

            fprintf(stderr,"%s : %s \n",__FUNCTION__, msgbuffer);
            TRACE_ERR(msgbuffer);
        }
        else
        {
            TRACE_DBG1(" : _21_ server write(%d,%s,%d) result=%d ",
                    fdServer,
                    a_pDataService->filenameServer,
                    vSize,result);
            result = 0;
        }

        close(fdServer);
        fdServer = -1;

        result = sem_post(a_pDataService->pSemsvc);
    }


    TRACE_DBG1(" : _3_ sem_post result=%d err=%d %s \n",
            result,errno,strerror(errno));

    //*********************************************************
    //      waiting receive response : polling
    //*********************************************************
    if( 0 == result )
    {
        vPollfdClient.events = POLLIN | POLLPRI ;
        vPollfdClient.revents = 0;
        vTimeout = -1;

        errno = 0;
        result  = poll(&vPollfdClient, vNfds, vTimeout);

        TRACE_DBG1(" : _31_ poll  result=%d: revents=%d 0x%X",
                result,
                (int)vPollfdClient.revents,(int)vPollfdClient.revents);


        if( ( -1 == result ) )
        {
            snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                    " : _32_ poll() errno=%d %s",
                    errno,strerror(errno));

            result = errno;

            fprintf(stderr,"%s : %s \n",__FUNCTION__, msgbuffer);
            TRACE_ERR(msgbuffer);

        }
        else if( 0 == result )
        {
            TRACE_DBG1(" : _33_ poll() Error Timeout %d %s ",
                    errno,strerror(errno));
            result = errno;
        }
        else if ( result && ( vPollfdClient.revents & POLLHUP) )
        {
            if( vPollfdClient.revents & POLLIN)
            {
                result = 0;
            }
            else
            {
                TRACE_ERR(" : _34_ poll() result=%d POLLHUP event",
                        result);

                result = EPIPE;
            }
        }
        else
        {
            result = 0;
        }
    }

    //*********************************************************
    //          read response from server
    //*********************************************************
    if( 0 == result )
    {

        errno = 0;
        vSize = sizeof(a_pDataService->response);
        result = read(  vPollfdClient.fd,
                        &a_pDataService->response,
                        vSize);

        if( 0 == result )
        {
            snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                    " : _4_ read(%d,%s,%d) Error client fifo result == 0  ",
                    vPollfdClient.fd,
                    a_pDataService->request.filenameClient,
                    vSize);

            fprintf(stderr,"%s : %s \n",__FUNCTION__, msgbuffer);
            TRACE_ERR(msgbuffer);
        }
        else if (-1 == result )
        {
            snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                    ": _41_ read(%d,%s,%d) error client %d %s",
                    vPollfdClient.fd,
                    a_pDataService->request.filenameClient,
                    vSize,errno,strerror(errno));

            fprintf(stderr,"%s : %s \n",__FUNCTION__, msgbuffer);
            TRACE_ERR(msgbuffer);
        }
        else
        {
            TRACE_DBG1(" : _42_ client fifo read from server size=%d result=%d\n",
                    vSize , result);
            result = 0;
        }
    }

    if( -1 != fdServer)
        close(fdServer);

    if( -1 != vPollfdClient.fd)
        close(vPollfdClient.fd);

    return result;
}

//****************************************************
//*
//*
//****************************************************
void * libmessage_threadFunction_srv(void * a_pArg)
{
    int             result          = 0;
    int             resultService   = 0;
    sDataThreadCtx_t   *pContext = (sDataThreadCtx_t*)a_pArg;

    int     fdServer = -1;
    int     fdClient = -1;
    struct pollfd   vPollfd = {0};
    nfds_t          vNfds    = 1;
    int             vTimeout = -1;
    int             sizebuffer = 0;
    char msgbuffer[APISYSLOG_MSG_SIZE] = {0};

    result = libmessage_mkfifo(pContext->dataService.filenameServer);
    if( 0 != result )
    {
        snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                " : error libmessage_mkfifo(%s)",
                pContext->dataService.filenameServer);

        fprintf(stderr,"%s : %s \n",__FUNCTION__, msgbuffer);
        TRACE_ERR(msgbuffer);
    }

    //******************************************************
    //  lock fifo server
    //******************************************************
    result = sem_wait(pContext->dataService.pSemsvc);
    TRACE_DBG1(" : _1_ sem_wait() result=%d err=%d %s \n",
            result,errno,strerror(errno));

    do
    {
        result = 0;
        //***************************************************
        //              open fifo server
        //***************************************************
        close(fdServer);

        errno = 0;
        fdServer = open(pContext->dataService.filenameServer,O_NONBLOCK|O_CLOEXEC|O_RDONLY);

        if( -1 == fdServer )
        {
            snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                    " :_2_ open(%s) err=%d %s \n",
                    pContext->dataService.filenameServer,errno,strerror(errno));
            result = errno;

            fprintf(stderr,"%s : %s \n",__FUNCTION__, msgbuffer);
            TRACE_ERR(msgbuffer);
        }

        if( 0 == result )
        {
            //***************************************************
            //              unlock
            //***************************************************
            result = sem_post(pContext->dataService.pSemsvc);
            TRACE_DBG1(" : _3_ sem_post() result=%d err=%d %s \n",
                    result,errno,strerror(errno));
        }

        if( 0 == result )
        {
            //***************************************************
            //              poll
            //***************************************************
            vPollfd.fd = fdServer;
            vPollfd.events = POLLIN | POLLPRI ;
            vPollfd.revents = 0;
            errno = 0;

            result  = poll(&vPollfd, vNfds, vTimeout);

            TRACE_DBG1(" : _4_ poll  result=%d: revents=%d 0x%X \n",
                    result, (int)vPollfd.revents,(int)vPollfd.revents);
        }
        if( 0 < result )
        {
            //******************************************************
            //  lock fifo server
            //******************************************************
            result = sem_wait(pContext->dataService.pSemsvc);
            TRACE_DBG1(" : _5_ sem_wait() result=%d err=%d %s \n",
                    result,errno,strerror(errno));
            //

        }
        if( 0 == result )
        {
            //***************************************************
            // read client request
            // client send filename for response
            //***************************************************
            sizebuffer =  sizeof(pContext->dataService.request);
            memset(&pContext->dataService.request,0,sizebuffer);

            errno = 0;
            result = read(fdServer, &pContext->dataService.request,sizebuffer);

            if( 0 == result )
            {
                snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                        " : _6_ read(%d,%s,%d) err=%d result == 0 ",
                        fdServer,
                        pContext->dataService.request.filenameClient,
                        sizebuffer, errno);
                fprintf(stderr,"%s : %s \n",__FUNCTION__, msgbuffer);
                TRACE_ERR(msgbuffer);

                result = -1;
            }
            else if (-1 == result )
            {
                snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                        " : _7_ read(%d,%s,%d) Error %d %s ",
                        fdServer,
                        pContext->dataService.request.filenameClient,
                        sizebuffer,errno,strerror(errno));

                fprintf(stderr,"%s : %s \n",__FUNCTION__, msgbuffer);
                TRACE_ERR(msgbuffer);
            }
            else
            {
                TRACE_DBG1(" : _8_ read(%d,%s,%d) result=%d",
                        fdServer,
                        pContext->dataService.request.filenameClient,
                        sizebuffer,result);
                result = 0;
            }
        }
        //*********************************************************
        //          open client endpoint
        //*********************************************************
        if( 0 == result )
        {
            errno = 0;
            fdClient = open(pContext->dataService.request.filenameClient,
                    O_NONBLOCK|O_CLOEXEC|O_WRONLY); //

            if( -1 == fdClient  )
            {
                snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                        " : _9_ open(%s) err=%d %s ",
                        pContext->dataService.request.filenameClient
                        ,errno,strerror(errno));
                result = errno;
                fprintf(stderr,"%s : %s \n",__FUNCTION__, msgbuffer);
                TRACE_ERR(msgbuffer);
            }
        }

        //*********************************************************
        //          call callback
        //*********************************************************
        if( 0 == result )
        {
            memset(&pContext->dataService.response,0,
                    sizeof(pContext->dataService.response));

            resultService = pContext->dataService.pFunctCB(
                  &pContext->dataService.request,
                  &pContext->dataService.response);

            (void)resultService;
        }
        //*********************************************************
        //          write response
        //*********************************************************
        if( 0 == result )
        {
            sizebuffer = sizeof(pContext->dataService.response);
            result = write(fdClient,
                    &pContext->dataService.response,
                    sizebuffer);
            if(-1 ==  result)
            {
                snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                        " : _10_ write(%d,%s,%d) err=%d %s \n",
                        fdClient,
                        pContext->dataService.request.filenameClient,
                        sizebuffer, errno,strerror(errno));
                fprintf(stderr,"%s : %s \n",__FUNCTION__, msgbuffer);
                TRACE_ERR(msgbuffer);

            }
            else
            {
                TRACE_DBG1(" :_11_ write(%d,%d) ok result=%d \n",
                        fdClient,sizebuffer,result);
            }
        }

        close(fdClient);

    }while(1);

    return (void*)0;
}


//******************************************************
//
//******************************************************
int libmessage_mkfifo(const char* a_endpointName)
{
    int result = 0;
    char msgbuffer[APISYSLOG_MSG_SIZE] = {0};

    unlink(a_endpointName);

    //*********************************************************
    // create server endpoint
    //*********************************************************
    errno = 0;
    result = mkfifo(a_endpointName,S_IRWXU);

    if( (0 != result ) && (EEXIST != errno) )
    {
        // error
        snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                ": mkfifo(-%s-) Error=%d %s \n",
                a_endpointName,
                errno,strerror(errno));

        fprintf(stderr,"%s : %s \n",__FUNCTION__, msgbuffer);
        TRACE_ERR(msgbuffer);

    }
    else
    {
        TRACE_DBG1(": mkfifo(%s) OK \n",
                a_endpointName);
        result = 0;
    }

    return result;
}
//******************************************************
//
// returnfd openned
//                  > stderr(2) : fd open
//                  -1 : error
//******************************************************
int libmessage_openfifo(    const char *a_Fifoname,
        uint32_t    a_flag ,
        int         *a_pFd)
{
    int result = 0;
    char msgbuffer[APISYSLOG_MSG_SIZE] = {0};

    // open client endpoint
    errno = 0;
    result = open(a_Fifoname, O_NONBLOCK|O_CLOEXEC|a_flag);

    if( -1 == result  )
    {
        snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                ":Error open(%s,0x%X) %d %s",
                a_Fifoname, O_NONBLOCK|O_CLOEXEC|a_flag,
                errno,strerror(errno));

        fprintf(stderr,"%s : %s \n",__FUNCTION__, msgbuffer);
        TRACE_ERR(msgbuffer);

    }
    else
    {
        TRACE_DBG1(": open(%s,0x%X) = %d \n",
                a_Fifoname, O_NONBLOCK|O_CLOEXEC|a_flag,result);
        *a_pFd = result;
        result = 0;
    }

    return result;
}


//******************************************************
//
//******************************************************

//******************************************************
//
//******************************************************



