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


#include "libmessage_int.h"
#include "libmessage.h"



static sDataThreadCtx_t g_DataThread[LIBMESSAGE_SRVID_END] = {0};

//*****************************************************************
//*
//*****************************************************************
sDataThreadCtx_t* getThreadContext(uint32_t a_ThreadCtx)
{
    sDataThreadCtx_t *pThreadContext = 0;

    if( LIBMESSAGE_SRVID_END > a_ThreadCtx  )
    {
        pThreadContext = &g_DataThread[a_ThreadCtx];
    }

    return pThreadContext ;
}
//*****************************************************************
//*
//*****************************************************************
sDataService_t *getDataservice(uint32_t a_ThreadCtx, uint32_t a_DataServiceID)
{
    sDataService_t * pDataservice  = 0;
    sDataThreadCtx_t *pDataThreadCtx = getThreadContext(a_ThreadCtx);

    if( ( pDataThreadCtx ) &&  (LIBMESSAGE_SRVID_TIME == a_ThreadCtx))
    {
        if( LIBMESSAGE_SVCID_TIME_END > a_DataServiceID )
        {
            pDataservice = &pDataThreadCtx->arrayDataService[a_DataServiceID];
        }
    }
    else if( ( pDataThreadCtx ) &&  (LIBMESSAGE_SRVID_NETWORK == a_ThreadCtx))
    {
        if( LIBMESSAGE_SVCID_NET_END > a_DataServiceID )
        {
            pDataservice = &pDataThreadCtx->arrayDataService[a_DataServiceID];
        }
    }
    else // LIBMESSAGE_SRVID_END
    {

    }

    return pDataservice ;
}
//*****************************************************************
//*
//*****************************************************************
pollfd_t *getPollfd(uint32_t a_ThreadCtx, uint32_t a_DataServiceID)
{
    pollfd_t * pPollfd = 0;

    sDataThreadCtx_t *pDataThreadCtx = getThreadContext(a_ThreadCtx);

    if( ( pDataThreadCtx ) &&  (LIBMESSAGE_SRVID_TIME == a_ThreadCtx))
    {
        if( LIBMESSAGE_SVCID_TIME_END > a_DataServiceID )
        {
            pPollfd = &pDataThreadCtx->arrayPollfd[a_DataServiceID];
        }
    }
    else if( ( pDataThreadCtx ) &&  (LIBMESSAGE_SRVID_NETWORK == a_ThreadCtx))
    {
        if( LIBMESSAGE_SVCID_NET_END > a_DataServiceID )
        {
            pPollfd = &pDataThreadCtx->arrayPollfd[a_DataServiceID];
        }
    }
    else // LIBMESSAGE_SRVID_END
    {

    }

    return pPollfd;
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
//int libmessage_register_service(
//        uint32_t    a_ServerID ,   // LIBMESSAGE_SRVID_TIME  LIBMESSAGE_SRVID_NETWORK
//        uint32_t    a_ServiceID ,   // SERVER_TIME_ID_GETDATE, SERVER_TIME_ID_SETDATE SERVER_TIME_ID_xxx
//        pFuncCB_t   a_pFuncCB)
//
//{
//    int result  = 0;
////    struct mq_attr  vAttr   = {0};
//
//    sDataThread_t *pContext = 0;
//
//    //***********************************************************************************
//    // Service time
//    //***********************************************************************************
//
//    if( LIBMESSAGE_SRVID_TIME == a_ServerID )
//    {
//        pContext = &g_DataThread[a_ServerID];
//
//        if( //(a_ServiceID >= SERVER_TIME_ID_GETDATE ) &&
//                (a_ServerID <= SERVER_TIME_ID_SIGNAL ))
//
//        result = libmessage_register_serviceID(pContext,a_ServiceID,a_pFuncCB);
//
//    }// if( LIBMESSAGE_SRVID_TIME == a_ServerID )
//
//
//
//
//    return result;
//}

//************************************************************
//*
//************************************************************
//int libmessage_register_serviceID(
//        sDataThreadCtx_t *a_pContext,
//        uint32_t    a_ServiceID ,   // SERVER_TIME_ID_GETDATE, SERVER_TIME_ID_SETDATE SERVER_TIME_ID_xxx
//        pFuncCB_t   a_pFuncCB )
//{
//    int result = 0;
//    struct mq_attr  vAttr   = {0};
//
//    if( 0 != a_pContext )
//    {
//        //*****************************************************************
//        //  add service in list
//        //*****************************************************************
//        strncpy(a_pContext->arrayDataService[a_ServiceID].filenameServer,
//                get_arrayServiceName(a_ServiceID),NAME_MAX) ;
//
//        a_pContext->arrayDataService->pFuncCB = a_pFuncCB;
//    } // if( 0 != pContext )
//
//
//    //*****************************************************************
//    // open MQ server
//    //*****************************************************************
//
//    vAttr.mq_flags  = O_CLOEXEC;
//    vAttr.mq_curmsgs = 9;
//    vAttr.mq_maxmsg = 9;
//    vAttr.mq_msgsize = 1024;
//    errno           = 0;
//    a_pContext->arrayPollfd[a_ServiceID].fd =
//            mq_open(a_pContext->arrayDataService[a_ServiceID].filenameServer,
//                    O_CREAT,S_IRWXO | S_IRWXG | S_IRWXU ,&vAttr);
//
//    if( a_pContext->arrayPollfd[a_ServiceID].fd  == ( (mqd_t)(-1) ))
//    {   //  error
//        result = errno;
//        printf("libmessage_getdate: mq_open(%s) error %d  %s\n",
//                a_pContext->arrayDataService[a_ServiceID].filenameServer,
//                result,strerror(result));
//    }
//    else
//    {
//        printf("libmessage_getdate: mq_open(%s) OK\n",
//                a_pContext->arrayDataService[a_ServiceID].filenameServer);
//    }
//
//
//    a_pContext->nbItem++;
//
//    return result;
//}
//****************************************************
//*
//*
//****************************************************
//static int libmessage_sendEvent(sDataThreadCtx_t *a_pContext, uint32_t a_IndexService )
//{
//    int     result      = 0;
//    int     vLenReceive = 0;
//    char    vBuffer[LIBMESSAGE_MAX_BUFFER] = {0};
//
//    vLenReceive =  mq_receive(a_pContext->arrayPollfd[a_IndexService].fd,
//            vBuffer,
//            sizeof(vBuffer),
//            0U);
//
//    result = a_pContext->arrayDataService[a_IndexService].pFuncCB(vBuffer);
//
//
//    printf("libmessage_sendEvent: vLenReceive=%d data=%s",vLenReceive,vBuffer);
//
//    return result;
//}
//****************************************************
//*
//*
//****************************************************
static void * libmessage_threadFunction(void * a_pArg)
{
    int             result  = 0;
    sDataThreadCtx_t   *pContext = (sDataThreadCtx_t*)a_pArg;
    int ii = 0;

//    do
//    {
//        // wait condition variable
//        // si list vide , wait condition variable
//        // sinon epoll sur MQ ouvert
//
//
//        //libmessage_m
//        printf("libmessage_threadFunction: poll before \n");
//        result = poll(pContext->arrayPollfd, pContext->nbItem, -1);
//        printf("libmessage_threadFunction: poll result=%d \n",result);
//
//        if ( result > 0 )
//        {
//            printf("libmessage_threadFunction: poll result > 0 \n");
//
//            for( ii = 0; ii < pContext->nbItem; ii ++ )
//            {
//                if(0 != pContext->arrayPollfd[ii].revents)
//                {
//                    printf("libmessage_threadFunction: revents != 0 \n");
//                    result = libmessage_sendEvent(pContext,ii);
//                }
//            }
////            result = libmessage_pollCheck();
//            printf("libmessage_threadFunction: result = libmessage_sendEvent =%d \n",result);
//        }
//        else if (0 == result )
//        {
//            // timeout
//            printf("libmessage_threadFunction: timeout result == 0 \n");
//       }
//        else // error
//        {
//            printf("libmessage_threadFunction: timeout result == error \n");
//
//        }
//        printf("libmessage_threadFunction: before while \n");
//
//    }while(1);


    return 0;
}


//****************************************************
//*
//*
//****************************************************
//int libmessage_init()
//{
//    int result = 0;
//
//    //*****************************
//    // create new tread for listening incomming messages
//    //*****************************
//    result =  pthread_create(   &gDataThread.ThreadID,
//                                NULL,
//                                &libmessage_threadFunction,
//                                0);
//
//
//    return result;
//}
//****************************************************
//*
//*
//****************************************************
//int libmessage_close()
//{
//    int result = 0;
//
//
//    result = pthread_cancel(gDataThread.ThreadID);
//
//
//    return result;
//}

//****************************************************
//*
//*
//****************************************************
//int libmessage_server_wait()
//{
//    int result = 0;
//
//    //*****************************
//    // LIBMESSAGE_ID_TIME
//    //*****************************
//    result =  pthread_create(   &(g_DataThread[LIBMESSAGE_SRVID_TIME].pthreadID),
//                                NULL,
//                                &libmessage_threadFunction,
//                                (void*)&g_DataThread[LIBMESSAGE_SRVID_TIME]);
//
//    result = pthread_join(g_DataThread[LIBMESSAGE_SRVID_TIME].pthreadID,0);
//
//
//    //*****************************
//    // LIBMESSAGE_ID_END
//    //*****************************
//
//    // wait on end of thread
//    return result;
//}
//****************************************************
//*
//*
//****************************************************
//int libmessage_client_register()
//{
//    int result = 0;
//
//    return result;
//}
////****************************************************
////*
////*
////****************************************************
//int libmessage_pollCheck()
//{
//    int result = 0;
//
//    return result;
//}

//******************************************************
//
//******************************************************
int libmessage_mkfifo(const char* a_endpointName)
{
    int result = 0;

    //*********************************************************
    // create server endpoint
    //*********************************************************
    errno = 0;
    result = mkfifo(a_endpointName,S_IRWXU);

    if( (0 != result ) && (EEXIST != errno) )
    {
        // error
        printf("%s libmessage_mkfifo: mkfifo(-%s-) Error=%d %s \n",
                getStrDate(),a_endpointName,errno,strerror(errno));
    }
    else
    {
        printf("%s libmessage_mkfifo :  -%s- OK \n",getStrDate(),a_endpointName);
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
        printf("%s libmessage_openfifo: Error %d %s: open(%s,0x%X)\n",
                getStrDate(),errno,strerror(errno),a_Fifoname,
                O_NONBLOCK|O_CLOEXEC|a_flag);
    }
    else
    {
        printf("%s libmessage_openfifo: open(%s,0x%X) = %d\n",
                getStrDate(),a_Fifoname, O_NONBLOCK|O_CLOEXEC|a_flag , result);
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

//******************************************************
//  server time
//******************************************************
int libmessage_srvtime_init()
{
    int result = 0;

    //***************************************************
    // register all services
    //***************************************************

    result = libmessage_srvtime_register_getdate();
//    result = libmessage_srvtime_svc_setdate();
//    result = libmessage_srvtime_svc_signal();

    //***************************************************
    // create thread listener
    //***************************************************



    return result;
}


