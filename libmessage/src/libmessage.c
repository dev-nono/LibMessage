/*
 * libmessage.c
 *
 *  Created on: 5 juin 2020
 *      Author: bruno
 */


#include <errno.h>
#include <pthread.h>
<<<<<<< HEAD
#include <string.h>
#include <stdio.h>

#include "libmessage_int.h"
#include "libmessage.h"



static sDataThread_t g_DataThread[LIBMESSAGE_SRVID_END] = {0};


sDataThread_t* getThreadContext(uint32_t a_ID)
{
    sDataThread_t *pThreadContext = 0;

    if( LIBMESSAGE_SRVID_END > a_ID  )
    {
        pThreadContext = &g_DataThread[a_ID];
    }


    return pThreadContext ;
}

//****************************************************
//*
//*
//****************************************************
int libmessage_register_service(
        uint32_t    a_ServerID ,   // LIBMESSAGE_SRVID_TIME  LIBMESSAGE_SRVID_NETWORK
        uint32_t    a_ServiceID ,   // SERVER_TIME_ID_GETDATE, SERVER_TIME_ID_SETDATE SERVER_TIME_ID_xxx
        pFuncCB_t   a_pFuncCB)

{
    int result  = 0;
//    struct mq_attr  vAttr   = {0};

    sDataThread_t *pContext = 0;

    //***********************************************************************************
    // Service time
    //***********************************************************************************

    if( LIBMESSAGE_SRVID_TIME == a_ServerID )
    {
        pContext = &g_DataThread[a_ServerID];

        if( //(a_ServiceID >= SERVER_TIME_ID_GETDATE ) &&
                (a_ServerID <= SERVER_TIME_ID_SIGNAL ))

        result = libmessage_register_serviceID(pContext,a_ServiceID,a_pFuncCB);

    }// if( LIBMESSAGE_SRVID_TIME == a_ServerID )




    return result;
}

//************************************************************
//*
//************************************************************
int libmessage_register_serviceID(
        sDataThread_t *a_pContext,
        uint32_t    a_ServiceID ,   // SERVER_TIME_ID_GETDATE, SERVER_TIME_ID_SETDATE SERVER_TIME_ID_xxx
        pFuncCB_t   a_pFuncCB )
{
    int result = 0;
    struct mq_attr  vAttr   = {0};

    if( 0 != a_pContext )
    {
        //*****************************************************************
        //  add service in list
        //*****************************************************************
        strncpy(a_pContext->arrayDataService[a_ServiceID].filenameServer,
                get_arrayServiceName(a_ServiceID),NAME_MAX) ;

        a_pContext->arrayDataService->pFuncCB = a_pFuncCB;
    } // if( 0 != pContext )


    //*****************************************************************
    // open MQ server
    //*****************************************************************

    vAttr.mq_flags  = O_CLOEXEC;
    vAttr.mq_curmsgs = 9;
    vAttr.mq_maxmsg = 9;
    vAttr.mq_msgsize = 1024;
    errno           = 0;
    a_pContext->arrayPollfd[a_ServiceID].fd =
            mq_open(a_pContext->arrayDataService[a_ServiceID].filenameServer,
                    O_CREAT,S_IRWXO | S_IRWXG | S_IRWXU ,&vAttr);

    if( a_pContext->arrayPollfd[a_ServiceID].fd  == ( (mqd_t)(-1) ))
    {   //  error
        result = errno;
        printf("libmessage_getdate: mq_open(%s) error %d  %s\n",
                a_pContext->arrayDataService[a_ServiceID].filenameServer,
                result,strerror(result));
    }
    else
    {
        printf("libmessage_getdate: mq_open(%s) OK\n",
                a_pContext->arrayDataService[a_ServiceID].filenameServer);
    }


    a_pContext->nbItem++;

    return result;
}
//****************************************************
//*
//*
//****************************************************
static int libmessage_sendEvent(sDataThread_t *a_pContext, uint32_t a_IndexService )
{
    int     result      = 0;
    int     vLenReceive = 0;
    char    vBuffer[LIBMESSAGE_MAX_BUFFER] = {0};

    vLenReceive =  mq_receive(a_pContext->arrayPollfd[a_IndexService].fd,
            vBuffer,
            sizeof(vBuffer),
            0U);

    result = a_pContext->arrayDataService[a_IndexService].pFuncCB(vBuffer);


    printf("libmessage_sendEvent: vLenReceive=%d data=%s",vLenReceive,vBuffer);

    return result;
}
//****************************************************
//*
//*
//****************************************************
static void * libmessage_threadFunction(void * a_pArg)
{
    int             result  = 0;
    sDataThread_t   *pContext = (sDataThread_t*)a_pArg;
    int ii = 0;

    do
    {
        // wait condition variable
        // si list vide , wait condition variable
        // sinon epoll sur MQ ouvert


        //libmessage_m
        printf("libmessage_threadFunction: poll before \n");
        result = poll(pContext->arrayPollfd, pContext->nbItem, -1);
        printf("libmessage_threadFunction: poll result=%d \n",result);

        if ( result > 0 )
        {
            printf("libmessage_threadFunction: poll result > 0 \n");

            for( ii = 0; ii < pContext->nbItem; ii ++ )
            {
                if(0 != pContext->arrayPollfd[ii].revents)
                {
                    printf("libmessage_threadFunction: revents != 0 \n");
                    result = libmessage_sendEvent(pContext,ii);
                }
            }
//            result = libmessage_pollCheck();
            printf("libmessage_threadFunction: result = libmessage_sendEvent =%d \n",result);
        }
        else if (0 == result )
        {
            // timeout
            printf("libmessage_threadFunction: timeout result == 0 \n");
       }
        else // error
        {
            printf("libmessage_threadFunction: timeout result == error \n");

        }
        printf("libmessage_threadFunction: before while \n");

    }while(1);


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
int libmessage_server_wait()
{
    int result = 0;

    //*****************************
    // LIBMESSAGE_ID_TIME
    //*****************************
    result =  pthread_create(   &(g_DataThread[LIBMESSAGE_SRVID_TIME].pthreadID),
                                NULL,
                                &libmessage_threadFunction,
                                (void*)&g_DataThread[LIBMESSAGE_SRVID_TIME]);

    result = pthread_join(g_DataThread[LIBMESSAGE_SRVID_TIME].pthreadID,0);


    //*****************************
    // LIBMESSAGE_ID_END
    //*****************************

    // wait on end of thread
    return result;
}
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

=======

// libmessage.h
#include <limits.h> // libmessage.h

#include "libmessage.h"
#include "libmessage_int.h"


struct sdataThread
{
    pthread_t       ThreadID;
    pthread_attr_t  Attr;


};

typedef struct sdataThread dataThread_t;

static dataThread_t gDataThread = {0};

//****************************************************
//*
//*
//****************************************************
int libmessage_manageMessage(const char *a_Message)
{
    int result = 0;

    if( 0 == *a_Message)
    {
        result = EINVAL;
    }



    return result;
}
//****************************************************
//*
//*
//****************************************************
static void * libmessage_threadFunction(void * a_pArg)
{
    (void) a_pArg;

    do
    {
        // wait condition variable
        // si list vide , wait condition variable
        // sinon epoll sur MQ ouvert


        //libmessage_m


    }while(1);


    return 0;
}
//****************************************************
//*
//*
//****************************************************
int libmessage_init()
{
    int result = 0;

    //*****************************
    // create new tread for listening incomming messages
    //*****************************
    result =  pthread_create(   &gDataThread.ThreadID,
                                NULL,
                                &libmessage_threadFunction,
                                0);


    return result;
}
//****************************************************
//*
//*
//****************************************************
int libmessage_close()
{
    int result = 0;


    result = pthread_cancel(gDataThread.ThreadID);


    return result;
}

//****************************************************
//*
//*
//****************************************************
int libmessage_server_register(const char * a_ServiceName)
{
    int result = 0;

    // find struct service

    // add service in list

    // notify thread

    return result;
}
//****************************************************
//*
//*
//****************************************************
int libmessage_server_wait()
{
    int result = 0;

    result = pthread_join(gDataThread.ThreadID,0);

    // wait on end of thread
    return result;
}
//****************************************************
//*
//*
//****************************************************
int libmessage_client_register()
{
    int result = 0;

    return result;
}
>>>>>>> branch 'master' of https://github.com/dev-nono/LibMessage.git

