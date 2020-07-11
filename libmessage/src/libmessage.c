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
#include "libmessage_int.h"
#include "libmessage.h"


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
    char    vClientName[NAME_MAX] = {0};
    int     fdServer = -1;
    char    dataBuffer[PIPE_BUF];
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
    snprintf(vClientName,vSize,"/tmp/client.%s",a_pDataService->filenameClient);

    //result = unlink(vClientName);

    result = libmessage_mkfifo(vClientName);

    //*********************************************************
    //          open client endpoint
    //*********************************************************
    if( 0 == result )
    {
        result = libmessage_openfifo(vClientName,O_RDONLY,&vPollfdClient.fd );
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

            fprintf(stderr,"%s : %s",__FUNCTION__, msgbuffer);
            TRACE_ERR(msgbuffer);
        }
    }

    //*********************************************************
    //          write request
    //*********************************************************
    if( 0 == result )
    {
        //send request to server endpoint
        errno = 0;
        result = write(fdServer,vClientName,strlen(vClientName));

        if(-1 ==  result)
        {
            result = errno;
            snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                    " : _2_ server write(-%s-) Error %d %s \n",
                    a_pDataService->filenameServer,errno,strerror(errno));

            fprintf(stderr,"%s : %s",__FUNCTION__, msgbuffer);
            TRACE_ERR(msgbuffer);
        }
        else
        {
            TRACE_DBG1(" : _21_ server write(%s) result=%d \n",
                    a_pDataService->filenameServer, result);
            result = 0;
        }

        close(fdServer);
        fdServer = -1;

        result = sem_post(a_pDataService->pSemsvc);
    }


    TRACE_DBG1(" : _3_ sem_post resulty=%d err=%d %s \n",
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

        TRACE_DBG1(" : _31_ poll  result=%d: revents=%d 0x%X \n",
                result,
                (int)vPollfdClient.revents,(int)vPollfdClient.revents);


        if( ( -1 == result ) )
        {
            snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                    " : _32_ poll() errno=%d %s \n",
                    errno,strerror(errno));

            result = errno;

            fprintf(stderr,"%s : %s",__FUNCTION__, msgbuffer);
            TRACE_ERR(msgbuffer);

        }
        else if( (result ) && ( vPollfdClient.revents & POLLHUP) )
        {
            TRACE_ERR(" : _33_ poll() result=%d POLLHUP event \n",
                    result);

            result = EPIPE;
        }
        else if( 0 == result )
        {
            TRACE_DBG1(" : _34_ poll() Error Timeout %d %s \n",
                    errno,strerror(errno));
            result = errno;
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

        memset(dataBuffer,0,sizeof(dataBuffer));
        errno = 0;
        result = read(vPollfdClient.fd,dataBuffer,PIPE_BUF);

        if( 0 == result )
        {
            snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                    " : _4_ read(-%s-) Error client %d:  result == 0  \n",
                    vClientName,result);

            fprintf(stderr,"%s : %s",__FUNCTION__, msgbuffer);
            TRACE_ERR(msgbuffer);
        }
        else if (-1 == result )
        {
            snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                    ": _41_ read(-%s-) error client %d %s \n",
                    vClientName,errno,strerror(errno));

            fprintf(stderr,"%s : %s",__FUNCTION__, msgbuffer);
            TRACE_ERR(msgbuffer);
        }
        else
        {
            TRACE_DBG1(" : _42_ client read server   size=%d \n",result);

            memcpy(a_pDataService->databuffer,dataBuffer,result);
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
    int             result  = 0;
    sDataThreadCtx_t   *pContext = (sDataThreadCtx_t*)a_pArg;

    char    vClientName[NAME_MAX] = {0};
    int     fdServer = -1;
    int     fdClient = -1;
    char    buffer[PIPE_BUF] = {0};
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

        fprintf(stderr,"%s : %s",__FUNCTION__, msgbuffer);
        TRACE_ERR(msgbuffer);
    }

    //******************************************************
    //  lock fifo server
    //******************************************************
    result = sem_wait(pContext->dataService.pSemsvc);
    TRACE_DBG1(" : _1_ sem_wait() result=%d err=%d %s \n",
            result,errno,strerror(errno));
    //
    //    //******************************************************
    //    //  release semaphore
    //    //******************************************************
    //    result = sem_post(pContext->dataService.pSemsvc);
    //
    //

    do
    {
        result = 0;

        memset(vClientName,0,sizeof(vClientName));
        // open server endpoint

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
                    FILENAME_SVC_TIME_GETDATE,errno,strerror(errno));
            result = errno;

            fprintf(stderr,"%s : %s",__FUNCTION__, msgbuffer);
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
            memset(vClientName,0,sizeof(vClientName));
            errno = 0;
            result = read(fdServer,vClientName,1024);


            if( 0 == result )
            {
                snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                        " : _6_ read(-%s-) err=%d size == 0  \n",
                        vClientName, errno);
                fprintf(stderr,"%s : %s",__FUNCTION__, msgbuffer);
                TRACE_ERR(msgbuffer);

                result = -1;
            }
            else if (-1 == result )
            {
                snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                        " : _7_ read(-%s-) Error %d %s \n",
                        vClientName,errno,strerror(errno));

                fprintf(stderr,"%s : %s",__FUNCTION__, msgbuffer);
                TRACE_ERR(msgbuffer);
            }
            else
            {
                TRACE_DBG1(" : _8_ read(-%s-) size=%d \n",
                        vClientName,result);
                result = 0;
            }
        }
        //*********************************************************
        //          open client endpoint
        //*********************************************************
        if( 0 == result )
        {
            errno = 0;
            fdClient = open(vClientName,O_NONBLOCK|O_CLOEXEC|O_WRONLY); //

            if( -1 == fdClient  )
            {
                snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                        " : _9_ open(-%s-) err=%d %s \n",
                        vClientName,errno,strerror(errno));
                result = errno;
                fprintf(stderr,"%s : %s",__FUNCTION__, msgbuffer);
                TRACE_ERR(msgbuffer);
            }
        }

        //*********************************************************
        //          call callback
        //*********************************************************
        if( 0 == result )
        {
            sizebuffer = pContext->dataService.pFunctCB(buffer);
        }
        //*********************************************************
        //          write response
        //*********************************************************
        if( 0 == result )
        {
            result = write(fdClient,buffer,sizebuffer);
            if(-1 ==  result)
            {
                snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                        " : _10_ write(-%s-) err=%d %s \n",
                        FILENAME_SVC_TIME_GETDATE,errno,strerror(errno));
                fprintf(stderr,"%s : %s",__FUNCTION__, msgbuffer);
                TRACE_ERR(msgbuffer);

            }
            else
            {
                TRACE_DBG1(" :_11_ write() ok len=%d \n",sizebuffer);
            }
        }

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

        fprintf(stderr,"%s : %s",__FUNCTION__, msgbuffer);
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

        fprintf(stderr,"%s : %s",__FUNCTION__, msgbuffer);
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



