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
#include <math.h>

#include <fcntl.h>           /* Pour les constantes O_* */
#include <sys/stat.h>        /* Pour les constantes des modes */
#include <pthread.h>
#include <signal.h>

#include <sys/types.h>          /* Consultez NOTES */
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>


#include "apisyslog.h"
#include "utilstools_network.h"

#include "libmsg_int.h"
#include "libmsg.h"


/**
 * \fn      int libmsg_cli_getdata(sDataService_t *a_pDataService,int a_unlinkFilenameclient)
 * \brief   client call this function for send request and receive response from server
 *
 *
 * \param       sDataService_t *a_pDataService  : input/output pointer struct
 *              : before fill
 *              - dataService.request.filenameClient   : client socket for server to write output
 *                  use getUniqname(...) for unique name
 *              - dataService.filenameServer            : filename server service to request
 *              - dataService.resquest                  : input request
 *              - dataService.request.header.datasize   : size of data send
 *              - dataService.request.pData             : input custom data send to server
 *
 * \param       int a_unlinkFilenameclient  : for receive signal do not unlink socket
 *              0       : not unlink socket
 *              != 0    : unlink socket
 *
 * \return      0 for OK
 *              dataService.response is filled
 *              dataService.response.header.result   : error code from remote service
 *              dataService.response.header.datasize : size of data send by server
 *              dataService.response.pData           : custom data send by server
 *
 *              or error code when sendig message
 *              - EINVAL
 *              - ...
 */
int libmsg_cli_getdata(sDataService_t *a_pDataService, int a_unlinkFilenameclient)
{
    int result      = 0;
    int sizeBuffer  = 0;
    int sock_client = -1;

    TRACE_DBG1(" _1 :");

    if((        !a_pDataService
            ||  0 == *a_pDataService->filenameServer
            ||  0 == *a_pDataService->request.filenameClient))
    {
        result = EINVAL;
    }

    if( 0 == result)
    {
        if( 0 != a_unlinkFilenameclient )
            unlink(a_pDataService->request.filenameClient);

        result = net_openBindConnect(   a_pDataService->request.filenameClient,
                a_pDataService->filenameServer,
                &sock_client);

        TRACE_DBG1(" _2 : openBindConnect(%s,%s,%d) result=%d ",
                a_pDataService->request.filenameClient,
                a_pDataService->filenameServer,sock_client,result);
    }
    if( 0 == result)
    {
        sizeBuffer = sizeof(sRequestClient_t);
        result = write(sock_client,&a_pDataService->request,sizeBuffer);

        TRACE_DBG1("_3 : write(%d,%d) result=%d",
                sock_client,sizeBuffer,result);
        if( -1 == result )
        {
            TRACE_DBG1("_4 : write(%d,%d) result=%d errno=%d %s",
                    sock_client,sizeBuffer,result,
                    errno , strerror(errno));

            result = errno;
        }
        else
        {
            result = 0;
        }
    }

    if( 0 == result)
    {
        TRACE_DBG1(" _41 : read(%d,%lu)",sock_client,sizeof(sResponse_t),result);
        result = read(sock_client,&a_pDataService->response,sizeof(sResponse_t));

        TRACE_DBG1(" _5 : read(%d,%lu) result=%d ",
                sock_client,sizeof(sResponse_t),result);

        if( -1 == result )
        {
            TRACE_DBG1("_6 : read(%d) result=%d errno=%d %s",
                    sock_client,result,errno , strerror(errno));
            result = errno;
        }
        else
            result = 0;
    }


    if( 0 != a_unlinkFilenameclient )
    {
        close(sock_client);
        unlink(a_pDataService->request.filenameClient);
    }
    else
    {
        a_pDataService->dataSignal.socket = sock_client;

        strncpy(a_pDataService->dataSignal.filenameClient,
                a_pDataService->request.filenameClient,
                sizeof(a_pDataService->dataSignal.filenameClient));

        net_resetConnectSocket(sock_client);
    }

    return result;
}

/**
 * \fn          static void * libmsg_srv_threadFunction(void * a_pArg)
 * \brief
 *
 * \param
 * \return
 */

static void * libmsg_srv_threadFunction(void * a_pArg)
{
    int             result          = 0;

    sThreadDataCtx_t *pContext = (sThreadDataCtx_t *)a_pArg;

    sRequestServer_t    request     = {0};
    sResponse_t         response    = {0};

    int                     sock_srv_read  = -1;

    struct sockaddr_storage peer_addr       = {0};
    socklen_t               peer_addr_len   = 0;
    int                     result_recvfrom = 0;

    TRACE_DBG1(" : _IN_1");

    result = unlink(pContext->dataService.filenameServer);

    TRACE_DBG1(" _2_ : unlink(%s) = %d  errno=%d %s",
            pContext->dataService.filenameServer,
            result,errno,strerror(errno) );

    result = net_openBind(pContext->dataService.filenameServer,&sock_srv_read);

    if( 0 != result)
    {
        TRACE_DBG1("%s : _3_ openBind(%s) = %d sock_srv_read=%d errno=%d %s",
                __FUNCTION__,pContext->dataService.filenameServer,
                result,sock_srv_read,errno,strerror(errno) );
    }

    if( 0 == result)
    {
        do
        {
            if( 0 == result)
            {
                result = net_resetConnectSocket(sock_srv_read);
//                usleep(1e6/1000); //1ms
            }

            if( 0 == result)
            {
                memset(&request,0,sizeof(sRequestServer_t));
                memset(&response,0,sizeof(sResponse_t));

                TRACE_DBG1("_31_ : before read(%d,%s) ",
                        sock_srv_read,
                        pContext->dataService.filenameServer);
                //*******************************************************
                // read input
                //
                memset(&peer_addr,0,sizeof(struct sockaddr ));
                peer_addr_len = sizeof(struct sockaddr_storage);

                result_recvfrom = recvfrom(sock_srv_read, &request.request, sizeof(sRequestClient_t), 0,
                        (struct sockaddr *) &peer_addr, &peer_addr_len);


                TRACE_DBG1("_4_ : read(%d,%s) result=%d ",
                        sock_srv_read,pContext->dataService.filenameServer,result);

                if( -1 == result_recvfrom )
                {
                    TRACE_ERR(" : read(%d)=%d errn=%d %s ",
                            sock_srv_read,result,errno,strerror(errno) );
                    result = 0;
                    continue;               /* Ignore failed request */
                }
                else
                {
                    result = pContext->dataService.pFunctCB(
                            &request,
                            &response);
                    TRACE_DBG1("_42_ : pContext->dataService.pFunctCB()=%d ",result);
                }
            }
//            if( 0 == result)
//            {
//                result = net_ConnectSocketUnix(sock_srv_read,request.filenameClient);
//                TRACE_DBG1("_43_ : net_ConnectSocketUnix(%d;%s)=%d ",
//                        sock_srv_read,request.filenameClient,result);
//            }
            {
                char host[NI_MAXHOST], service[NI_MAXSERV];
                int result2 = 0;

                result2 = getnameinfo((struct sockaddr *) &peer_addr,
                        peer_addr_len, host, NI_MAXHOST,
                        service, NI_MAXSERV, NI_NUMERICSERV);
                if (0 == result)
                {
                    TRACE_DBG1(" : Received %d bytes from host=%s service=%s",result_recvfrom, host, service);
                }
                else
                {
                    TRACE_ERR(" : getnameinfo: %s", gai_strerror(result2));
                    result = 0;
                }
            }
            if( 0 == result )
            {
                //*******************************************************
                // write data to ouput socket
//                result = write(sock_srv_read,&response, max(sizeof(sResponse_t),response.header.datasize));

                   result =  sendto(sock_srv_read, &response, response.header.datasize, 0,
                           (struct sockaddr *) &peer_addr,peer_addr_len) ;

                TRACE_DBG1("_5_ : write(%d) result=%d errno=%d %s ",sock_srv_read,
                        result,errno,strerror(errno) );

                if( -1 == result)
                {
                    TRACE_ERR("_6_ : write(%d) result=%d errno=%d %s ",sock_srv_read,
                            result,errno,strerror(errno) );
                    result = errno;
                }
                else
                {
                    result = 0;
                }
            }

            TRACE_DBG1(" _6_ :  end loop ");

            result = 0;

            //        pcond_unlock(&g_Condition);
            //
            //        sizeList = tq_size(&g_listClient);
            //        printf("%s _8_ : sizeList=%d ",__FUNCTION__ ,sizeList);

        }while(1);

        close(sock_srv_read);

    }// // 0 = openBind(SRV_GETDATE,&sock_srv_read);

    return (void*)0;
}
/**
 * \fn      static void * libmsg_cli_threadFunction_signal(void * a_pArg)
 * \brief
 *
 * \param
 * \return
 */
static void * libmsg_cli_threadFunction_signal(void * a_pArg)
//****************************************************************************
{
    int             result          = 0;

    sThreadDataCtxSignal_t *pContext = (sThreadDataCtxSignal_t *)a_pArg;

    sRequestServer_t    requestServer         = {0};
    sResponse_t         response        = {0};
//    int                 sock_srv_read   = -1;
    char                host[NI_MAXHOST]  = {0};
    char                service[NAME_MAX] = {0}; // NI_MAXSERV
    int                 result_recvfrom   = 0;

    TRACE_DBG1(" : _IN_1");

    do
    {

        if( 0 == result)
        {
            memset(&requestServer, 0,sizeof(requestServer));
            memset(&response,0,sizeof(response));

            memset(host,   0,sizeof(host));
            memset(service,0,sizeof(service));


            TRACE_DBG1("_31_ : before read(%d,%s) ",pContext->dataService.dataSignal.socket
                    ,pContext->dataService.dataSignal.filenameClient);
            //*******************************************************
            // read input
            //
            requestServer.peer_addr_len = sizeof(requestServer.peer_addr);

            result_recvfrom = recvfrom(pContext->dataService.dataSignal.socket,
                    &response, sizeof(sResponse_t), 0,
                    (struct sockaddr *) &requestServer.peer_addr, &requestServer.peer_addr_len);


//            {
//                int result2 = 0;
//
//                result2 = getnameinfo(
//                        (struct sockaddr*)&request.peer_addr,
//                        request.peer_addr_len,
//                        host,       sizeof(host),
//                        service,    sizeof(service),
//                        NI_NUMERICHOST | NI_NUMERICSERV);
//                if (0 == result2)
//                {
//                    TRACE_DBG1("32_ : getnameinfo Received %d bytes from host=%s service=%s",result_recvfrom, host, service);
//                }
//                else
//                {
//                    TRACE_DBG1(" 33_: getnameinfo: %d %s", result2,gai_strerror(result2));
//                    //result = 0;
//                }
//            }
//
//
            TRACE_DBG1("_4_ : recvfrom()=%d ",result_recvfrom);

            if( -1 == result_recvfrom )
            {
                TRACE_ERR(" : recvfrom(%d)=%d errn=%d %s ",
                        pContext->dataService.dataSignal.socket,
                        result,errno,strerror(errno) );
                result = 0;
                continue;               /* Ignore failed request */
            }
            else
            {
                result = pContext->dataService.pFunctCBresponse(&response);
                TRACE_DBG1("_41_ : pContext->dataService.pFunctCB()=%d ",result);
            }
            if( 0 != result)
                result = -1;
        }


        TRACE_DBG1(" _6_ :  end loop ");

        result = 0;

        //        pcond_unlock(&g_Condition);
        //
        //        sizeList = tq_size(&g_listClient);
        //        printf("%s _8_ : sizeList=%d ",__FUNCTION__ ,sizeList);

    }while(1);

     return (void*)0;
}//static void * libmsg_cli_threadFunction_signal(void * a_pArg)
/**
 * \fn      static void * libmsg_srv_threadFunction_signal(void * a_pArg)
 * \brief
 *
 * \param
 * \return
 */
static void * libmsg_srv_threadFunction_signal(void * a_pArg)
//****************************************************************************
{
    int             result          = 0;

    sThreadDataCtxSignal_t *pContext = (sThreadDataCtxSignal_t *)a_pArg;

    sRequestServer_t    request         = {0};
    sResponse_t         response        = {0};
    int                 sock_srv_read   = -1;
    char                host[NI_MAXHOST]  = {0};
    char                service[NAME_MAX] = {0}; // NI_MAXSERV
    int                 result_recvfrom   = 0;

    TRACE_DBG1(" : _IN_1");

    result = unlink(pContext->dataService.filenameServer);

    TRACE_DBG1(" _2_ : unlink(%s) = %d  errno=%d %s",
            pContext->dataService.filenameServer,
            result,errno,strerror(errno) );

    result = net_openBind(pContext->dataService.filenameServer,&sock_srv_read);

    if( 0 != result)
    {
        TRACE_DBG1("%s : _3_ openBind(%s) = %d sock_srv_read=%d errno=%d %s",
                __FUNCTION__,pContext->dataService.filenameServer,
                result,sock_srv_read,errno,strerror(errno) );
    }

    if( 0 == result)
    {
        do
        {
            if( 0 == result)
            {
                result = net_resetConnectSocket(sock_srv_read);
            }

            if( 0 == result)
            {
                memset(&request, 0,sizeof(request));
                memset(&response,0,sizeof(response));

                memset(host,   0,sizeof(host));
                memset(service,0,sizeof(service));


                TRACE_DBG1("_31_ : before read(%d,%s) ",sock_srv_read,pContext->dataService.filenameServer);
                //*******************************************************
                // read input
                //
                request.peer_addr_len = sizeof(request.peer_addr);

                result_recvfrom = recvfrom(sock_srv_read, &request.request, sizeof(sRequestClient_t), 0,
                        (struct sockaddr *) &request.peer_addr, &request.peer_addr_len);


                {
                    int result2 = 0;

                    result2 = getnameinfo(
                            (struct sockaddr*)&request.peer_addr,
                            request.peer_addr_len,
                            host,       sizeof(host),
                            service,    sizeof(service),
                            NI_NUMERICHOST | NI_NUMERICSERV);
                    if (0 == result2)
                    {
                        TRACE_DBG1("32_ : getnameinfo Received %d bytes from host=%s service=%s",result_recvfrom, host, service);
                    }
                    else
                    {
                        TRACE_DBG1(" 33_: getnameinfo: %d %s", result2,gai_strerror(result2));
                        //result = 0;
                    }
                }


                TRACE_DBG1("_4_ : recvfrom(%d,%s) result=%d peer_addr_len=%d ss_family=%d",
                        sock_srv_read,host,
                        result_recvfrom,
                        (int)request.peer_addr_len,
                        (int)(((struct sockaddr *)&request.peer_addr)->sa_family));

                if( -1 == result_recvfrom )
                {
                    TRACE_ERR(" : read(%d)=%d errn=%d %s ",
                            sock_srv_read,result,errno,strerror(errno) );
                    result = 0;
                    continue;               /* Ignore failed request */
                }
                else
                {
                    result = pContext->dataService.pFunctCB(&request,&response);
                    TRACE_DBG1("_41_ : pContext->dataService.pFunctCB()=%d ",result);
                }
                if( 0 != result)
                    result = -1;
            }
            if( 0 == result)
            {
                result = net_ConnectSocketUnix(sock_srv_read,service);
                TRACE_DBG1("_43_ : net_ConnectSocketUnix(%d;%s)=%d ",
                        sock_srv_read,service,result);
            }
            if( 0 == result )
            {
                //*******************************************************
                // write data to ouput socket

                   result =  sendto(sock_srv_read, &response, response.header.datasize, 0,
                           (struct sockaddr *) &request.peer_addr,request.peer_addr_len) ;

                TRACE_DBG1("_5_ : sendto(%d) result=%d errno=%d %s ",sock_srv_read,
                        result,result >=0 ? 0: errno,strerror( result >=0 ? 0 : errno) );

                if( -1 == result)
                {
                    TRACE_ERR("_6_ : sendto(%d) result=%d errno=%d %s ",sock_srv_read,
                            result,errno,strerror(errno) );
                    result = errno;
                }
                else
                {
                    result = 0;
                }
            }

            TRACE_DBG1(" _6_ :  end loop ");

            result = 0;

            //        pcond_unlock(&g_Condition);
            //
            //        sizeList = tq_size(&g_listClient);
            //        printf("%s _8_ : sizeList=%d ",__FUNCTION__ ,sizeList);

        }while(1);

        close(sock_srv_read);

    }// // 0 = openBind(SRV_GETDATE,&sock_srv_read);

    return (void*)0;
}


/**
 * \fn      int libmsg_srv_register_svc(sThreadDataCtx_t *a_pThreadDataCtx)
 *
 * \brief   server side create generic thread dedicated for receive and send data from client
 *          when client use libmsg_cli_getdata(...) for call service
 *          - this thread read request from a_pThreadDataCtx->dataService.filenameServer
 *          - call a_pThreadDataCtx->dataService.pFunctCB for custom service
 *          - a_pThreadDataCtx->dataService.pFunctCB fiil response
 *          - this thread write response from callback to client connected to
 *              socket pThreadDataCtx->dataService.filenameServer
 *
 *
 * \param       sThreadDataCtx_t *a_pThreadDataCtx      : thread data context for one service
 *                  - fill a_pThreadDataCtx->dataService.pFunctCB with static method for custom service
 *                      use a_pRequest for input data
 *                      use a_pResponse for result of service
 *                  - fill a_pThreadDataCtx->dataService.filenameServer  with
 *                      name of socket for server service used by client to connect
 *
 * \return       error code  of pthread_create()
 */
int libmsg_srv_register_svc(sThreadDataCtx_t *a_pThreadDataCtx)
{
    int result = 0;
    char msgbuffer[APISYSLOG_MSG_SIZE] = {0};

    //*****************************
    // create new tread for listening incomming messages
    //*****************************
    result =  pthread_create(&a_pThreadDataCtx->pthreadID,
            NULL,
            &libmsg_srv_threadFunction,
            (void*)a_pThreadDataCtx);

    if( 0 != result )
    {
        snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                ": pthread_create() error =%d %s",
                result,strerror(result));

        TRACE_ERR(msgbuffer);
    }

    return result;
}
/**
 * \fn      int libmsg_srv_register_svc_Signal(sThreadDataCtxSignal_t *a_pThreadDataCtx)
 * \brief
 *
 * \param
 * \return
 */
int libmsg_srv_register_svc_Signal(sThreadDataCtxSignal_t *a_pThreadDataCtx)
{
    int result = 0;
    char msgbuffer[APISYSLOG_MSG_SIZE] = {0};

    //*****************************
    // create new tread for listening incomming messages
    //*****************************
    errno = 0;
    result =  pthread_create(&a_pThreadDataCtx->pthreadID,
            NULL,
            &libmsg_srv_threadFunction_signal,
            (void*)a_pThreadDataCtx);

    if( 0 != result )
    {
        snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                ": pthread_create() error =%d %s",
                result,strerror(result));

        TRACE_ERR(msgbuffer);
    }

    return result;
}
/**
 * \fn      int libmsg_cli_register_svc_Signal(sThreadDataCtxSignal_t *a_pThreadDataCtx)
 * \brief
 *
 * \param
 * \return
 */
int libmsg_cli_register_svc_Signal(sThreadDataCtxSignal_t *a_pThreadDataCtx)
{
    int result = 0;
    char msgbuffer[APISYSLOG_MSG_SIZE] = {0};

    //*****************************
    // create new tread for listening incomming messages
    //*****************************
    errno = 0;
    result =  pthread_create(&a_pThreadDataCtx->pthreadID,
            NULL,
            &libmsg_cli_threadFunction_signal,
            (void*)a_pThreadDataCtx);

    if( 0 != result )
    {
        snprintf(msgbuffer,APISYSLOG_MSG_SIZE-50,
                ": pthread_create() error =%d %s",
                result,strerror(result));

        TRACE_ERR(msgbuffer);
    }

    return result;
}
/**
 * \fn      sRequestServer_t * libsmg_dupRequestServer(sRequestServer_t *a_pRequestServer)
 *
 * \brief   duplicate sRequestServer_t  with calloc()
 *
 * \param   sRequestServer_t *a_pRequest : data to dulicate
 *
 * \return  pointer to data dulicated
 */
sRequestServer_t * libsmg_dupRequestServer(sRequestServer_t *a_pRequestServer)
{
    sRequestServer_t *pRequest = (sRequestServer_t *)calloc(1,sizeof(sRequestServer_t));

    memcpy(pRequest,a_pRequestServer,sizeof(sRequestServer_t));

    return pRequest;
}
