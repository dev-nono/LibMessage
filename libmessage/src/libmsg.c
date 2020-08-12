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


int libmsg_cli_getdata(sDataService_t *a_pDataService)
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
        sizeBuffer = sizeof(sRequest_t);
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

    close(sock_client);
    unlink(a_pDataService->request.filenameClient);

    return result;
}


static void * libmsg_srv_threadFunction(void * a_pArg)
{
    int             result          = 0;

    sThreadDataCtx_t *pContext = (sThreadDataCtx_t *)a_pArg;

    sRequest_t  request     = {0};
    sResponse_t response    = {0};

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
                memset(&request,0,sizeof(sRequest_t));
                memset(&response,0,sizeof(sResponse_t));

                TRACE_DBG1("_31_ : before read(%d,%s) ",sock_srv_read,pContext->dataService.filenameServer);
                //*******************************************************
                // read input
                //
                memset(&peer_addr,0,sizeof(struct sockaddr ));
                peer_addr_len = sizeof(struct sockaddr_storage);

                result_recvfrom = recvfrom(sock_srv_read, &request, sizeof(sRequest_t), 0,
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
//****************************************************************************
//*
//****************************************************************************
static void * libmsg_srv_threadFunction_signal(void * a_pArg)
//****************************************************************************
{
    int             result          = 0;

    sThreadDataCtxSignal_t *pContext = (sThreadDataCtxSignal_t *)a_pArg;

    sRequest_t  request     = {0};
    sResponse_t response    = {0};

    int                     sock_srv_read  = -1;

    sRequestTimer_t *pRequestsignal = 0;
    char    host[NI_MAXHOST];
    char    service[NAME_MAX]; // NI_MAXSERV

//    socklen_t               peer_addr_len;
//
//    struct sockaddr peer_addr1;
//    struct sockaddr_un peer_addr2;
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
            }

            if( 0 == result)
            {
                memset(&request,0,sizeof(sRequest_t));
                memset(&response,0,sizeof(sResponse_t));

                memset(host,0,sizeof(host));
                memset(service,0,sizeof(service));


                TRACE_DBG1("_31_ : before read(%d,%s) ",sock_srv_read,pContext->dataService.filenameServer);
                //*******************************************************
                // read input
                //
                pRequestsignal = (sRequestTimer_t *) calloc(1,sizeof(sRequestTimer_t)) ;

                pRequestsignal->peer_addr_len = sizeof(pRequestsignal->peer_addr);

                result_recvfrom = recvfrom(sock_srv_read, &request, sizeof(sRequest_t), 0,
                        (struct sockaddr *) &pRequestsignal->peer_addr, &pRequestsignal->peer_addr_len);


                {
                    int result2 = 0;

                    result2 = getnameinfo(
                            (struct sockaddr*)&pRequestsignal->peer_addr,
                            pRequestsignal->peer_addr_len,
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


                TRACE_DBG1("_4_ : read(%d,%s) result=%d peer_addr_len=%d ss_family=%d",
                        sock_srv_read,host,
                        result_recvfrom,
                        (int)pRequestsignal->peer_addr_len,
                        (int)(((struct sockaddr *)&pRequestsignal->peer_addr)->sa_family));

                if( -1 == result_recvfrom )
                {
                    TRACE_ERR(" : read(%d)=%d errn=%d %s ",
                            sock_srv_read,result,errno,strerror(errno) );
                    result = 0;

                    free(pRequestsignal);
                    pRequestsignal = 0;

                    continue;               /* Ignore failed request */
                }
                else
                {
                    memcpy(&pRequestsignal->request,&request,sizeof(pRequestsignal->request));

                    result = pContext->dataService.pFunctCB(
                            pRequestsignal,
                            &response);
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
                           (struct sockaddr *) &pRequestsignal->peer_addr,pRequestsignal->peer_addr_len) ;

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
//static void * libmsg_srv_threadFunction(void * a_pArg)
//{
//    int             result          = 0;
//
//    sDataThreadCtx_t *pContext = (sDataThreadCtx_t *)a_pArg;
//
//    sRequest_t  request     = {0};
//    sResponse_t response    = {0};
//
//    int sock_srv_read  = -1;
//
//    TRACE_DBG1(" : _IN_1");
//
//    result = unlink(pContext->dataService.filenameServer);
//
//    TRACE_DBG1(" _2_ : unlink(%s) = %d  errno=%d %s",
//            pContext->dataService.filenameServer,
//            result,errno,strerror(errno) );
//
//    result = libmsg_openBind(pContext->dataService.filenameServer,&sock_srv_read);
//
//    if( 0 != result)
//    {
//        TRACE_DBG1("%s : _3_ openBind(%s) = %d sock_srv_read=%d errno=%d %s",
//                __FUNCTION__,pContext->dataService.filenameServer,
//                result,sock_srv_read,errno,strerror(errno) );
//    }
//
//    if( 0 == result)
//    {
//        do
//        {
//            if( 0 == result)
//            {
//                result = net_resetConnectSocket(sock_srv_read);
////                usleep(1e6/1000); //1ms
//            }
//
//            if( 0 == result)
//            {
//                memset(&request,0,sizeof(sRequest_t));
//                memset(&response,0,sizeof(sResponse_t));
//
//                TRACE_DBG1("_31_ : before read(%d,%s) ",sock_srv_read,pContext->dataService.filenameServer);
//                //*******************************************************
//                // read input
//                //
//                result = read(sock_srv_read,&request,sizeof(sRequest_t));
//
//                TRACE_DBG1("_4_ : read(%d,%s) result=%d ",
//                        sock_srv_read,pContext->dataService.filenameServer,result);
//
//                if( -1 == result )
//                {
//                    TRACE_ERR(" : read(%d)=%d errn=%d %s ",
//                            sock_srv_read,result,errno,strerror(errno) );
//                    result = errno;
//                }
//                else
//                {
//                    result = pContext->dataService.pFunctCB(
//                            &request,
//                            &response);
//                    TRACE_DBG1("_42_ : pContext->dataService.pFunctCB()=%d ",result);
//                }
//            }
//            if( 0 == result)
//            {
//                result = net_ConnectSocketUnix(sock_srv_read,request.filenameClient);
//                TRACE_DBG1("_43_ : net_ConnectSocketUnix(%d;%s)=%d ",
//                        sock_srv_read,request.filenameClient,result);
//            }
//            if( 0 == result )
//            {
//                //*******************************************************
//                // write data to ouput socket
//                result = write(sock_srv_read,&response, util_max(sizeof(sResponse_t),response.header.datasize));
//                TRACE_DBG1("_5_ : write(%d) result=%d errno=%d %s ",sock_srv_read,
//                        result,errno,strerror(errno) );
//
//                if( -1 == result)
//                {
//                    TRACE_ERR("_6_ : write(%d) result=%d errno=%d %s ",sock_srv_read,
//                            result,errno,strerror(errno) );
//                    result = errno;
//                }
//                else
//                {
//                    result = 0;
//                }
//            }
//
//            TRACE_DBG1(" _6_ :  end loop ");
//
//            result = 0;
//
//            //        pcond_unlock(&g_Condition);
//            //
//            //        sizeList = tq_size(&g_listClient);
//            //        printf("%s _8_ : sizeList=%d ",__FUNCTION__ ,sizeList);
//
//        }while(1);
//
//        close(sock_srv_read);
//
//    }// // 0 = openBind(SRV_GETDATE,&sock_srv_read);
//
//    return (void*)0;
//}

//************************************************************
//*
//************************************************************
int libmsg_srv_register_svc(sThreadDataCtx_t *a_pThreadDataCtx)
{
    int result = 0;
    char msgbuffer[APISYSLOG_MSG_SIZE] = {0};

    //*****************************
    // create new tread for listening incomming messages
    //*****************************
    errno = 0;
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
//************************************************************
//*
//************************************************************
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
//***************************************************
// return   : -1 if find
//          :  0 if not find
//***************************************************
//int libmsg_srv_find_registred_client(
//        sRequest_t      *a_pRequest,
//        unsigned int    a_nfds,
//        const char      *a_filenameClient)
//{
//    int result = 0;
//    int unsigned ii = 0;
//
//    if( 0 == a_nfds )
//    {
//        result = 0;
//    }
//    else
//    {
//        for(ii=0;       (ii < (MAX_POLL_FD))
//        &&  (ii <= a_nfds)
//        &&  (0 < a_nfds);
//        ii++ )
//        {
//            result = strcmp(a_pRequest->filenameClient,a_filenameClient);
//
//            if( result == 0 )
//            {
//                result = -1;
//                break;
//            }
//        }
//
//        if( 0 == result)
//        {
//            result = -1;
//        }
//        else
//        {
//            result = 0;
//        }
//    }
//
//    return result;
//}
