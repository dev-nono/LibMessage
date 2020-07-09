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

#include "libmessage_int.h"
#include "libmessage.h"

#define NS_PER_SECOND (1000000000LL)


void add_timespec(struct timespec t1, struct timespec t2, struct timespec *td)
{
    td->tv_nsec = t2.tv_nsec + t1.tv_nsec;
    td->tv_sec  = t2.tv_sec + t1.tv_sec;
    if (td->tv_nsec >= NS_PER_SECOND)
    {
        td->tv_nsec -= NS_PER_SECOND;
        td->tv_sec++;
    }
    else if (td->tv_nsec <= -NS_PER_SECOND)
    {
        td->tv_nsec += NS_PER_SECOND;
        td->tv_sec--;
    }
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
//  return: 0 OK
//          error number
//************************************************************
int libmessage_svc_getdata(sDataService_t *a_pDataService)
{
    int     result = 0;
    char    vClientName[NAME_MAX] = {0};
    int     fdServer = -1;
    char    dataBuffer[PIPE_BUF];
    int     vSize = 0;

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

    struct timespec vdate = {0,0};
    struct timespec vdatedelta = {0,1e9/1000*100};
    struct timespec abs_timeout = {0,0};

    clock_gettime(CLOCK_REALTIME, &vdate);

    add_timespec(vdate,vdatedelta,&abs_timeout);

    //***************************************************
    //              lock
    //***************************************************
    result = sem_timedwait(a_pDataService->pSemsvc,&abs_timeout);
    printf("%s _1_ sem_wait() result=%d err=%d %s \n",
            getStrDate(),result,errno,strerror(errno));


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
            printf("%s %s: _2_ server write(-%s-) Error %d %s \n",
                    getStrDate(),__FUNCTION__,
                    a_pDataService->filenameServer,errno,strerror(errno));
            result = errno;
        }
        else
        {
            printf("%s %s: _21_ server write(%s) result=%d \n",
                    getStrDate(),__FUNCTION__,
                    a_pDataService->filenameServer, result);
            result = 0;
        }
            close(fdServer);
            fdServer = -1;

            result = sem_post(a_pDataService->pSemsvc);
   }


    fprintf(stderr,"%s %s: _3_ sem_post resulty=%d err=%d %s \n",
            getStrDate(),__FUNCTION__,
            result,errno,strerror(errno));

//    printf("%s _3_ type any key to continue 1 \n",getStrDate());
//    getchar();

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

        printf("%s %s: _31_ poll  result=%d: revents=%d 0x%X \n",
                getStrDate(),__FUNCTION__,
                result,
                (int)vPollfdClient.revents,(int)vPollfdClient.revents);


        if( ( -1 == result ) )
        {
            printf("%s %s: _32_ poll() errno=%d %s \n",
                    getStrDate(),__FUNCTION__,
                    errno,strerror(errno));
            result = errno;
        }
        else if( (result ) && ( vPollfdClient.revents & POLLHUP) )
        {
            printf("%s %s: _33_ poll() result=%d POLLHUP event \n",
                    getStrDate(),__FUNCTION__,result);

            result = EPIPE;
        }
        else if( 0 == result )
        {
            printf("%s %s: _34_ poll() Error Timeout %d %s \n",
                    getStrDate(),__FUNCTION__,
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
            printf("%s %s: _4_ read(-%s-) Error client %d:  result == 0  \n",
                    getStrDate(),__FUNCTION__,
                    vClientName,result);
        }
        else if (-1 == result )
        {
            printf("%s %s: _41_ read(-%s-) error client %d %s \n",
                    getStrDate(),__FUNCTION__,
                    vClientName,errno,strerror(errno));
        }
        else
        {
            printf("%s %s: _42_ client read server response = %s  size=%d \n",
                    getStrDate(),__FUNCTION__,
                    dataBuffer,result);

            memcpy(a_pDataService->databuffer,
                    dataBuffer,
                    sizeof(a_pDataService->databuffer));
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
const char * getStrDate()
{
    int result = 0;
    static char vBuffer[100] = {0};
    struct timespec vRes = {0,0};

    result = clock_gettime(CLOCK_MONOTONIC_RAW,&vRes);
    snprintf(vBuffer,25, "%4ld.%09ld",vRes.tv_sec,vRes.tv_nsec);

    return vBuffer;
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

    result = libmessage_mkfifo(pContext->dataService.filenameServer);
    if( 0 != result )
    {
        fprintf(stderr,"%s %s : error libmessage_mkfifo(%s)",
                getStrDate(),__FUNCTION__,pContext->dataService.filenameServer);
    }

    //******************************************************
    //  lock fifo server
    //******************************************************
    result = sem_wait(pContext->dataService.pSemsvc);
    printf("%s %s : _1_ sem_wait() result=%d err=%d %s \n",
            getStrDate(),__FUNCTION__,result,errno,strerror(errno));
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
            printf("%s %s:_2_ open(%s) err=%d %s \n",
                    getStrDate(),__FUNCTION__,
                    SVCNAME_TIME_GETDATE,errno,strerror(errno));
            result = errno;
        }

        if( 0 == result )
        {
            //***************************************************
            //              unlock
            //***************************************************
            result = sem_post(pContext->dataService.pSemsvc);
            printf("%s %s: _3_ sem_post() result=%d err=%d %s \n",
                    getStrDate(),__FUNCTION__,result,errno,strerror(errno));
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

            printf("%s %s: _4_ poll  result=%d: revents=%d 0x%X \n",
                    getStrDate(),__FUNCTION__,result, (int)vPollfd.revents,(int)vPollfd.revents);
        }
        if( 0 < result )
        {
            //******************************************************
            //  lock fifo server
            //******************************************************
            result = sem_wait(pContext->dataService.pSemsvc);
            printf("%s %s : _5_ sem_wait() result=%d err=%d %s \n",
                    getStrDate(),__FUNCTION__,result,errno,strerror(errno));
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
                printf("%s %s: _6_ read(-%s-) err=%d size == 0  \n",
                        getStrDate(),__FUNCTION__,
                        vClientName, errno);
                result = -1;
            }
            else if (-1 == result )
            {
                printf("%s %s : _7_ read(-%s-) Error %d %s \n",
                        getStrDate(),__FUNCTION__,
                        vClientName,errno,strerror(errno));
            }
            else
            {
                printf("%s %s _8_ read(-%s-) size=%d \n",
                        getStrDate(),__FUNCTION__,
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
                printf("%s %s _9_ open(-%s-) err=%d %s \n",
                        getStrDate(),__FUNCTION__,
                        vClientName,errno,strerror(errno));
                result = errno;
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
                printf("%s %s: _10_ write(-%s-) err=%d %s \n",
                        getStrDate(),__FUNCTION__,
                        SVCNAME_TIME_GETDATE,errno,strerror(errno));
            }
            else
            {
                printf("%s %s:_11_ write(%s) ok len=%d \n",
                        getStrDate(),__FUNCTION__,
                        buffer,sizebuffer);
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

    unlink(a_endpointName);

    //*********************************************************
    // create server endpoint
    //*********************************************************
    errno = 0;
    result = mkfifo(a_endpointName,S_IRWXU);

    if( (0 != result ) && (EEXIST != errno) )
    {
        // error
        fprintf(stderr,"%s %s: mkfifo(-%s-) Error=%d %s \n",
                getStrDate(),__FUNCTION__,
                a_endpointName,errno,strerror(errno));
    }
    else
    {
        fprintf(stderr,"%s %s : mkfifo(%s) OK \n",
                getStrDate(),__FUNCTION__,a_endpointName);
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

    // open client endpoint
    errno = 0;
    result = open(a_Fifoname, O_NONBLOCK|O_CLOEXEC|a_flag);

    if( -1 == result  )
    {
        fprintf(stderr,"%s %s:Error open(%s,0x%X) %d %s\n",
                getStrDate(),__FUNCTION__,
                a_Fifoname, O_NONBLOCK|O_CLOEXEC|a_flag,
                errno,strerror(errno));
    }
    else
    {
        fprintf(stderr,"%s %s: open(%s,0x%X) = %d \n",
                getStrDate(),__FUNCTION__,
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



