/*
 * libmessage.c
 *
 *  Created on: 5 juin 2020
 *      Author: bruno
 */


#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>

#include <unistd.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <unistd.h>
#include <math.h>

#include "apisyslog.h"
#include <utils.h>

#include "libmessage_int.h"







//************************************************************
//*
//************************************************************
int libmessage_server_register_svc(sDataThreadCtx_t *a_pDataThreadCtx)
{
    int result = 0;
 //   sDataThreadCtx_t *pDataThreadCtx = a_pDataThreadCtx;
    char msgbuffer[APISYSLOG_MSG_SIZE] = {0};

    //*****************************
    // prepare data thread
    //*****************************
//    if( 0 == result )
//    {
//        pDataThreadCtx = getTheadCtx(a_MessageID);
//    }

    //**************************************************
    //*  create semaphore
    //**************************************************
    if( 0 == result )
    {
        errno = 0;
        result = sem_unlink(a_pDataThreadCtx->dataService.filenameSemaphore);
        if( 0 != result )
        {
            snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                    ": sem_unlink(%s) result=%d errno=%d %s",
                    a_pDataThreadCtx->dataService.filenameSemaphore,
                    result,errno,strerror(errno));

            fprintf(stderr,"%s : %s \n",__FUNCTION__, msgbuffer);
            TRACE_ERR(msgbuffer);
        }

        result = 0;
        errno = 0;
        a_pDataThreadCtx->dataService.pSemsvc = sem_open(
                a_pDataThreadCtx->dataService.filenameSemaphore,
                O_CREAT,S_IRWXU,1U);
        if( SEM_FAILED == a_pDataThreadCtx->dataService.pSemsvc)
        {
            snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                    "sem_open(%s) result=0x%p errno=%d %s",
                    a_pDataThreadCtx->dataService.filenameSemaphore,
                    (void*)a_pDataThreadCtx->dataService.pSemsvc,
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
        result =  pthread_create(&a_pDataThreadCtx->pthreadID,
                NULL,
                &libmessage_server_threadFunction,
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
////************************************************************
////*
////************************************************************
//int libmessage_srvtime_register_svc(    eLIBMSG_ID_t            a_MessageID,
//                                        libmessage_pFunctCB_t   a_pFunctCB )
//{
//    int result = 0;
//    sDataThreadCtx_t *pDataThreadCtx = 0;
//    char msgbuffer[APISYSLOG_MSG_SIZE] = {0};
//
//    if( (a_MessageID < 0) || (a_MessageID >=  eLIBMSG_ID_END) )
//    {
//        result = EINVAL;
//    }
//
//    //*****************************
//    // prepare data thread
//    //*****************************
//    if( 0 == result )
//    {
//        pDataThreadCtx = getTheadCtx(a_MessageID);
//    }
//
//    //*****************************
//    // prepare data thread
//    //*****************************
//    if( 0 == result )
//    {
//        pDataThreadCtx->dataService.pFunctCB = a_pFunctCB;
//
//        strncpy(pDataThreadCtx->dataService.filenameServer,
//                getNameService(a_MessageID,eLIBMSG_COL_SRV_FILENAME),
//                sizeof(pDataThreadCtx->dataService.filenameServer)-1);
//    }
//
//    //**************************************************
//    //*  create semaphore
//    //**************************************************
//    if( 0 == result )
//    {
//        errno = 0;
//        result = sem_unlink(getNameService(a_MessageID,eLIBMSG_COL_SEM));
//        if( 0 != result )
//        {
//            snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
//                    ": sem_unlink(%s) result=%d errno=%d %s",
//                    getNameService(a_MessageID,eLIBMSG_COL_SEM),
//                    result,errno,strerror(errno));
//
//            fprintf(stderr,"%s : %s \n",__FUNCTION__, msgbuffer);
//            TRACE_ERR(msgbuffer);
//        }
//
//        result = 0;
//        errno = 0;
//        pDataThreadCtx->dataService.pSemsvc = sem_open(
//                getNameService(a_MessageID,eLIBMSG_COL_SEM),
//                O_CREAT,S_IRWXU,1U);
//        if( SEM_FAILED == pDataThreadCtx->dataService.pSemsvc)
//        {
//            snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
//                    "sem_open(%s) result=0x%p errno=%d %s",
//                    getNameService(a_MessageID,eLIBMSG_COL_SEM),
//                    (void*)pDataThreadCtx->dataService.pSemsvc,
//                    errno,strerror(errno));
//
//            fprintf(stderr,"%s : %s\n",__FUNCTION__, msgbuffer);
//            TRACE_ERR(msgbuffer);
//        }
//    }
//
//    if( 0 == result )
//    {
//       //*****************************
//        // create new tread for listening incomming messages
//        //*****************************
//        errno = 0;
//        result =  pthread_create(&pDataThreadCtx->pthreadID,
//                NULL,
//                &libmessage_threadFunction_srv,
//                (void*)pDataThreadCtx);
//
//        if( 0 != result )
//        {
//
//            snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
//                    ": pthread_create() error =%d %s",
//                    result,strerror(result));
//
//            fprintf(stderr,"%s : %s \n",__FUNCTION__, msgbuffer);
//            TRACE_ERR(msgbuffer);
//        }
//    }
//    return result;
//}

//************************************************************
//
//************************************************************
int libmessage_waitSemaphoreAtDate(sem_t *a_pSemaphore , long long unsigned a_nano)
{
    int     result      = 0;
    char    msgbuffer[APISYSLOG_MSG_SIZE] = {0};

    struct timespec vdatedelta = {0,a_nano}; // 100ms
    struct timespec abs_timeout = {0,0};

    ADD_TIMESPEC_REAL(vdatedelta,abs_timeout);

    TRACE_DBG1(" : sem_timedwait()  nano=%lu",a_nano);

    //result = sem_wait(a_pSemaphore);

    //***************************************************
    //              lock
    //***************************************************
    result = sem_timedwait(a_pSemaphore,&abs_timeout);
    if( 0 != result )
    {
        snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                " : sem_timedwait() result=%d err=%d %s",
                result,errno,strerror(errno));

        fprintf(stderr,"%s : %s \n",__FUNCTION__, msgbuffer);
        TRACE_ERR(msgbuffer);
    }
    TRACE_DBG1(" : sem_timedwait() result=%d nano=%lu",result, a_nano);

    return result;
}
//************************************************************
//
//************************************************************
int libmessage_writeRequest(sDataService_t *a_pDataService,int a_fileDescriptor)
{
    int     result      = 0;
    char    msgbuffer[APISYSLOG_MSG_SIZE] = {0};
    int     vSize       = 0;

    //send request to server endpoint
    vSize = a_pDataService->request.header.datasize;
    errno = 0;
    result = write(a_fileDescriptor,
            (char*)&a_pDataService->request,
             vSize);

    if(-1 ==  result)
    {
        result = errno;
        snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                " : server write(%d,%s,%d) Error %d %s ",
                a_fileDescriptor,
                a_pDataService->filenameServer,
                vSize,errno,strerror(errno));

        fprintf(stderr,"%s : %s \n",__FUNCTION__, msgbuffer);
        TRACE_ERR(msgbuffer);
    }
    else
    {
        TRACE_DBG1(" : server write(%d,%s,%d) result=%d ",
                a_fileDescriptor,
                a_pDataService->filenameServer,
                vSize,result);
        result = 0;
    }

    close(a_fileDescriptor);
    a_fileDescriptor = -1;

    //if( SUCCESS == result)
    {
        errno = 0;
        result = sem_post(a_pDataService->pSemsvc);

        if( SUCCESS != result)
        {
            result = errno;
            snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                    " : sem_post() Error %d %s ",
                    errno,strerror(errno));
        }
    }

    return result;
}
//************************************************************
//
//************************************************************
int libmessage_pollResponse(int a_fileDescriptor)
{
    int             result      = 0;
    char            msgbuffer[APISYSLOG_MSG_SIZE] = {0};
    struct pollfd   vPollfdClient = {0};
    nfds_t          vNfds    = 1;
    int             vTimeout = -1;

    vPollfdClient.fd      = a_fileDescriptor;
    vPollfdClient.events = POLLIN | POLLPRI ;
    vPollfdClient.revents = 0;
    vTimeout = -1;

    errno = 0;
    do{
        vPollfdClient.revents = 0;
        result  = poll(&vPollfdClient, vNfds, vTimeout);

        TRACE_DBG1(" : poll(%d) result=%d: revents=%d 0x%X",
                a_fileDescriptor,result,
                (int)vPollfdClient.revents,(int)vPollfdClient.revents);

    }while( vPollfdClient.revents & POLLNVAL);

    TRACE_DBG1(" : poll_2(%d) result=%d: revents=%d 0x%X",
            a_fileDescriptor,result,
            (int)vPollfdClient.revents,(int)vPollfdClient.revents);


    if( ( -1 == result ) )
    {
        snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                " : poll(%d) result=%d errno=%d %s",
                a_fileDescriptor,result,errno,strerror(errno));

        result = errno;

        fprintf(stderr,"%s : %s \n",__FUNCTION__, msgbuffer);
        TRACE_ERR(msgbuffer);

    }
    else if( 0 == result )
    {
        TRACE_DBG1(" : poll(%d) result=%d Error Timeout %d %s ",
                a_fileDescriptor,result,errno,strerror(errno));
        result = errno;
    }
    else if ( result && ( vPollfdClient.revents & POLLHUP) )
    {
        if( vPollfdClient.revents & POLLIN)
        {
            result = 0;
            TRACE_DBG1(" : poll(%d) result=%d POLLIN=0x%X",
                    a_fileDescriptor,result,POLLIN);
       }
        else
        {
            TRACE_ERR(" : poll(%d) result=%d POLLHUP event",
                    a_fileDescriptor,result);

            result = EPIPE;
        }
    }
    else
    {
        result = 0;
    }

    return result;
}
//************************************************************
//
//************************************************************
int libmessage_readDataResponse(sDataService_t *a_pDataService,
        int a_fileDescriptor)
{
    int     result      = 0;
    char    msgbuffer[APISYSLOG_MSG_SIZE] = {0};
    int     vSize       = 0;

    errno = 0;
    vSize = sizeof(a_pDataService->response);
    result = read(  a_fileDescriptor,
                    &a_pDataService->response,
                    vSize);

    if( 0 == result )
    {
        snprintf(msgbuffer,APISYSLOG_MSG_SIZE-1,
                " : read(%d,%.50s,%d) Error client fifo result == 0  ",
                a_fileDescriptor,
                a_pDataService->request.filenameClient,
                vSize);

        fprintf(stderr,"%s : %s \n",__FUNCTION__, msgbuffer);
        TRACE_ERR(msgbuffer);
    }
    else if (-1 == result )
    {
        snprintf(msgbuffer,APISYSLOG_MSG_SIZE-1,
                ": read(%d,%.50s,%d) error client %d %s",
                a_fileDescriptor,
                a_pDataService->request.filenameClient,
                vSize,errno,strerror(errno));

        fprintf(stderr,"%s : %s \n",__FUNCTION__, msgbuffer);
        TRACE_ERR(msgbuffer);
    }
    else
    {
        TRACE_DBG1(" : client fifo read from server size=%d result=%d\n",
                vSize , result);
        result = 0;
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
int libmessage_client_getdataFromServer(sDataService_t *a_pDataService)
{
    int     result      = 0;
    int     fdServer    = -1;
    int     vPollfdClient = -1;
    //char    msgbuffer[APISYSLOG_MSG_SIZE] = {0};

    //******************************************
    //* wait semaphore at new date
    //******************************************
    if( 0 == result )
    { // // 10ms
        result = libmessage_waitSemaphoreAtDate(a_pDataService->pSemsvc ,1e9/100);
    }
    TRACE_DBG1(" : _1_ libmessage_waitSemaphoreAtDate result=%d \n",result);

    //*********************************************************
    //          create client fifo
    //*********************************************************

    if( 0 == result )
    {
        result = libmessage_mkfifo(a_pDataService->request.filenameClient);
    }
    //*********************************************************
    //          open client endpoint
    //*********************************************************
    if( 0 == result )
    {
        result = libmessage_openfifo(a_pDataService->request.filenameClient,
                //O_WRONLY,
                //O_RDWR,
                O_RDONLY,
                &vPollfdClient );
    }

    //*********************************************************
    //          open server fifo
    //*********************************************************
    if( 0 == result )
    {
        // open server endpoint  argv[1]
        fdServer = -1;
        result = libmessage_openfifo(a_pDataService->filenameServer,
                O_WRONLY,&fdServer);
    }

    //*********************************************************
    //          write request
    //*********************************************************
    if( 0 == result )
    {
         //send request to server endpoint
       result = libmessage_writeRequest(a_pDataService,fdServer);
    }
    TRACE_DBG1(" : _3_ sem_post result=%d err=%d %s \n",
            result,errno,strerror(errno));

    //******************************************
    //* wait semaphore at new date
    //******************************************
//    if( 0 == result )
//    { // // 100ms
//        result = libmessage_waitSemaphoreAtDate(a_pDataService->pSemsvc ,1e9/10);
//    }

    //*********************************************************
    //      waiting receive response : polling
    //*********************************************************
    if( 0 == result )
    {
        result = libmessage_pollResponse(fdServer);
    }
    //*********************************************************
    //          read response from server
    //*********************************************************
//    if( 0 == result )
//    {
//        TRACE_DBG1(" : _4_ libmessage_openfifo_block()\n");
//        result = libmessage_openfifo_block(a_pDataService->request.filenameClient,
//                //O_WRONLY,
//                //O_RDWR,
//                O_RDONLY,
//                &vPollfdClient );
//        TRACE_DBG1(" : _41_ libmessage_openfifo_block(%s) result=%d\n",
//                a_pDataService->request.filenameClient,
//                result);
//    }
    if( 0 == result )
    {
        result = libmessage_readDataResponse(a_pDataService,vPollfdClient);
    }

    if( -1 != fdServer)
        close(fdServer);

    if( -1 != vPollfdClient)
        close(vPollfdClient);

    unlink(a_pDataService->request.filenameClient);

    return result;
}

//****************************************************
//*
//*
//****************************************************
static void * libmessage_server_threadFunction(void * a_pArg)
{
    int             result          = 0;
    int             resultService   = 0;
    int             resultpoll      = 0;
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
    //result = libmessage_waitSemaphoreAtDate(pContext->dataService.pSemsvc,(1e9 / 100));

    result = sem_wait(pContext->dataService.pSemsvc);
    TRACE_DBG1(" : _1_ sem_wait() result=%d err=%d %s \n",
            result,errno,strerror(errno));

    do
    {
        if( 0 == result )
        {
            //***************************************************
            //              unlock
            //***************************************************
            result = sem_post(pContext->dataService.pSemsvc);
            TRACE_DBG1(" : _11_ sem_post() result=%d err=%d %s \n",
                    result,errno,strerror(errno));
        }

        result = 0;
        //***************************************************
        //              open fifo server
        //***************************************************
        close(fdServer);

        errno = 0;
        fdServer = open(pContext->dataService.filenameServer,
                //O_NONBLOCK|
                O_CLOEXEC|O_RDONLY);
        TRACE_DBG1(" : _12_ open(%s) result=%d err=%d %s \n",
                pContext->dataService.filenameServer,
                result,errno,strerror(errno));

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
//            result = sem_post(pContext->dataService.pSemsvc);
//            TRACE_DBG1(" : _3_ sem_post() result=%d err=%d %s \n",
//                    result,errno,strerror(errno));
        }

//        if( 0 == result )
//        {
//            //***************************************************
//            //              poll
//            //***************************************************
//            vPollfd.fd = fdServer;
//            vPollfd.events = POLLIN | POLLPRI ;
//            vPollfd.revents = 0;
//            errno = 0;
//
//            resultpoll  = poll(&vPollfd, vNfds, vTimeout);
//
//            TRACE_DBG1(" : _4_ poll  result=%d: revents=%d 0x%X \n",
//                    resultpoll, (int)vPollfd.revents,(int)vPollfd.revents);
//        }
//        if( ( 0 == result ) && (0 < resultpoll))
        if(  0 == result )
        {
            //******************************************************
            //  lock fifo server
            //******************************************************
//            result = libmessage_waitSemaphoreAtDate(pContext->dataService.pSemsvc,(1e9 / 100 ));
            //result = sem_wait(pContext->dataService.pSemsvc);
//            TRACE_DBG1(" : _5_ sem_wait() result=%d err=%d %s \n",
//                    result,errno,strerror(errno));


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
                        " : _6_ read(%d,%.50s,%d) err=%d result == 0 ",
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
                        " : _7_ read(%d,%.50s,%d) Error %d %s ",
                        fdServer,
                        pContext->dataService.request.filenameClient,
                        sizebuffer,errno,strerror(errno));

                fprintf(stderr,"%s : %s \n",__FUNCTION__, msgbuffer);
                TRACE_ERR(msgbuffer);
            }
            else
            {
                TRACE_DBG1(" : _8_ read(%d,%.50s,%d) result=%d",
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

            TRACE_DBG1(" : _9_ open(%s) err=%d %s ",
                    pContext->dataService.request.filenameClient,
                    errno,strerror(errno));

            if( -1 == fdClient  )
            {
                snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                        " : _91_ open(%.50s) err=%d %s ",
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

//            resultService = pContext->dataService.pFunctCB(
//                  &pContext->dataService.request,
//                  &pContext->dataService.response);
            resultService = pContext->dataService.pFunctCB(pContext);
            TRACE_DBG1(" : _92_ callCB resultService=%d",resultService);
            (void)resultService;
        }
        //*********************************************************
        //          write response
        //*********************************************************
        if( 0 == result )
        {
            sizebuffer = pContext->dataService.response.header.datasize;

            result = write(fdClient,
                    &pContext->dataService.response,
                    sizebuffer);

            TRACE_DBG1(" : _10_ write(%d,%.50s,%d) err=%d %s \n",
                    fdClient,
                    pContext->dataService.request.filenameClient,
                    sizebuffer, errno,strerror(errno));

            if(-1 ==  result)
            {
                snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                        " : _11_ write(%d,%.50s,%d) err=%d %s \n",
                        fdClient,
                        pContext->dataService.request.filenameClient,
                        sizebuffer, errno,strerror(errno));
                fprintf(stderr,"%s : %s \n",__FUNCTION__, msgbuffer);
                TRACE_ERR(msgbuffer);

            }
            else
            {
                TRACE_DBG1(" :_12_ write(%d,%d) ok result=%d \n",
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
int libmessage_openfifo_block(    const char *a_Fifoname,
        uint32_t    a_flag ,
        int         *a_pFd)
{
    int result = 0;
    char msgbuffer[APISYSLOG_MSG_SIZE] = {0};

    // open client endpoint
    errno = 0;
    result = open(a_Fifoname, O_CLOEXEC|a_flag);

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

//************************************************************
//*             incoming event signal
//************************************************************
static void * libmessage_client_threadFunction_signal(void * a_pArg)
{
    int             result          = SUCCESS;
    sDataThreadCtx_t  *pDataThreadCtx     = (sDataThreadCtx_t *)a_pArg;
    //int             vTimeout        = 500;
    int    ii              = 0;
    char        msgbuffer[APISYSLOG_MSG_SIZE] = {0};
    int             sizebuffer = 0;

    sigset_t    sigset_mask = {0};
    int         signal = 0;

    sigemptyset(&sigset_mask);
    sigaddset(&sigset_mask, SIGUSR1);
    sigprocmask(SIG_BLOCK, &sigset_mask, NULL);

    if( 0 == pDataThreadCtx->nfds )
    {
        result = sigwait(&sigset_mask, &signal);
    }

    do
    {
        result = poll(  pDataThreadCtx->pollFdClient,
                pDataThreadCtx->nfds,-1);

        if( -1 == result )
        {
            result = errno;

            snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50," : _1_ poll() errno=%d %s",errno,strerror(errno));
            fprintf(stderr,"%s : %s \n",__FUNCTION__, msgbuffer);
            TRACE_ERR(msgbuffer);
        }
        else if( EINTR  == result )
        {
            continue;
        }
        else if(0 == result)
        {
            // timeout
        }
        else
        {
            for(ii = 0; ii < result; ii++)
            {
                if( pDataThreadCtx->pollFdClient[ii].revents & POLLIN)
                {
                    sizebuffer =  sizeof(pDataThreadCtx->dataService.request);
                    memset(&pDataThreadCtx->dataService.request,0,sizebuffer);

                    errno = 0;
                    result = read(pDataThreadCtx->pollFdClient[ii].fd,
                            &pDataThreadCtx->dataService.request,
                            sizebuffer);

                    // call callback for signal
                    result = pDataThreadCtx->dataService.pFunctCB(pDataThreadCtx);
                }
            }

        }
    }while(1);

    return (void*)0;
}
//************************************************************
//*
//************************************************************
int libmessage_client_register_signal(sDataService_t *a_pDataService) // TODO
{
    int result = 0;



    return result;
}

//******************************************************
//
//******************************************************
int libmessage_client_createThreadSignal(sDataThreadCtx_t *a_pDataThreadCtx)
{
    int result = SUCCESS;
    char msgbuffer[APISYSLOG_MSG_SIZE] = {0};

    errno = 0;
    result =  pthread_create(&a_pDataThreadCtx->pthreadID,
            NULL,
            &libmessage_client_threadFunction_signal,
            (void*)a_pDataThreadCtx);

    if( SUCCESS != result )
    {
        snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                ": pthread_create() error =%d %s",
                result,strerror(result));

        fprintf(stderr,"%s : %s \n",__FUNCTION__, msgbuffer);
        TRACE_ERR(msgbuffer);
    }

    return result;
}
//******************************************************
//
//******************************************************
int libmessage_server_register_fifosignal(sDataThreadCtx_t *a_pDataThreadCtx) // TODO
{
    int result = SUCCESS;
    char msgbuffer[APISYSLOG_MSG_SIZE] = {0};

    errno = 0;

    // open fifo client
    //a_pDataThreadCtx->dataService.request.filenameClient;


//    if( SUCCESS != result )
//    {
//        snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
//                ": pthread_create() error =%d %s",
//                result,strerror(result));
//
//        fprintf(stderr,"%s : %s \n",__FUNCTION__, msgbuffer);
//        TRACE_ERR(msgbuffer);
//    }

    return result;
}



