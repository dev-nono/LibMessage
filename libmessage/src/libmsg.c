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

        result = libmsg_openBindConnect(   a_pDataService->request.filenameClient,
                a_pDataService->filenameServer,
                &sock_client);

        TRACE_DBG1(" _3 : openBindConnect(%s,%s,%d) result=%d ",
                a_pDataService->request.filenameClient,
                a_pDataService->filenameServer,sock_client,result);
    }
    if( 0 == result)
    {
        sizeBuffer = sizeof(sRequest_t);
        result = write(sock_client,&a_pDataService->request,sizeBuffer);

        TRACE_DBG1("_4 : write(%d,%d) result=%d",
                sock_client,sizeBuffer,result);
        if( -1 == result )
        {
            TRACE_DBG1("_5 : write(%d,%d) result=%d errno=%d %s",
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

    sDataThreadCtx_t *pContext = (sDataThreadCtx_t *)a_pArg;

    sRequest_t  request     = {0};
    sResponse_t response    = {0};

    int sock_srv_read  = -1;

    TRACE_DBG1(" : _IN_1");

    result = unlink(pContext->dataService.filenameServer);

    TRACE_DBG1(" _2_ : unlink(%s) = %d  errno=%d %s",
            pContext->dataService.filenameServer,
            result,errno,strerror(errno) );

    result = libmsg_openBind(pContext->dataService.filenameServer,&sock_srv_read);

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
                result = read(sock_srv_read,&request,sizeof(sRequest_t));

                TRACE_DBG1("_4_ : read(%d,%s) result=%d ",
                        sock_srv_read,pContext->dataService.filenameServer,result);

                if( -1 == result )
                {
                    TRACE_ERR(" : read(%d)=%d errn=%d %s ",
                            sock_srv_read,result,errno,strerror(errno) );
                    result = errno;
                }
                else
                {
                    result = pContext->dataService.pFunctCB(
                            &request,
                            &response);
                    TRACE_DBG1("_42_ : pContext->dataService.pFunctCB()=%d ",result);
                }
            }
            if( 0 == result)
            {
                result = net_ConnectSocketUnix(sock_srv_read,request.filenameClient);
                TRACE_DBG1("_43_ : net_ConnectSocketUnix(%d;%s)=%d ",
                        sock_srv_read,request.filenameClient,result);
            }
            if( 0 == result )
            {
                //*******************************************************
                // write data to ouput socket
                result = write(sock_srv_read,&response, max(sizeof(sResponse_t),response.header.datasize));
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

int libmsg_openBindConnect( const char  *a_clientFilename,
                            const char  *a_serverFilename,
                            int         *a_pSocketdescriptor)
{
    int result = 0;
    int Socketdescriptor = 0;
    int yes = 1;
    struct sockaddr_un bind_sockaddr = {0};

    if( (! (*a_clientFilename)) || !(*a_serverFilename) || !a_pSocketdescriptor)
    {
        result = EINVAL;
    }

    if( 0 == result)
    {
        Socketdescriptor = socket(AF_UNIX, SOCK_DGRAM, 0);

        if( -1 == Socketdescriptor )
        {
            TRACE_ERR(" : socket()=%d errno=%d %s ",Socketdescriptor,errno,strerror(errno) );
            result = errno;
        }
    }
    if( 0 == result )
    {
        result = setsockopt(Socketdescriptor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if( 0 != result )
        {
            TRACE_ERR(" : setsockopt()=%d errno=%d %s ",
                    result,errno,strerror(errno) );
            result = errno;
            close(Socketdescriptor);
        }
    }

    if( 0 == result)
    {
        unlink(a_clientFilename);

        /* Efface la structure */
        memset(&bind_sockaddr, 0, sizeof(struct sockaddr));

        bind_sockaddr.sun_family = AF_UNIX;
        strncpy(bind_sockaddr.sun_path, a_clientFilename,
                sizeof(bind_sockaddr.sun_path) - 1);

        result = bind(Socketdescriptor, (struct sockaddr *) &bind_sockaddr,
                sizeof(struct sockaddr_un)) ;

        if( -1 == result  )
        {
            fprintf(stderr,"%s : bind(%d)=%d errn=%d %s ",
                    __FUNCTION__,Socketdescriptor,result,errno,strerror(errno) );
            close(Socketdescriptor);
            result = errno;
        }
        else
        {
            *a_pSocketdescriptor = Socketdescriptor;
        }
    }

    if( 0 == result)
    {
        result = net_ConnectSocketUnix(Socketdescriptor,a_serverFilename);

        if( 0 !=  result )
            *a_pSocketdescriptor = -1;
    }

    return result;
}

int libmsg_openBind(const char *a_socketFilename,int* a_pSocketdescriptor)
{
    int result = 0;
    int Socketdescriptor = 0;
    int yes = 1;
    struct sockaddr_un sockaddr = {0};

    Socketdescriptor = socket(AF_UNIX, SOCK_DGRAM, 0);

    if( -1 == Socketdescriptor )
    {
        TRACE_ERR(" : socket()=%d errno=%d %s ",
                Socketdescriptor,errno,strerror(errno) );
        result = errno;
    }

    if( 0 == result )
    {
        result = setsockopt(Socketdescriptor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if( 0 != result )
        {
            TRACE_ERR(" : setsockopt()=%d errno=%d %s ",
                    result,errno,strerror(errno) );
            result = errno;
            close(Socketdescriptor);
        }
    }

    if( 0 == result)
    {
        /* Efface la structure */
        memset(&sockaddr, 0, sizeof(struct sockaddr_un));

        sockaddr.sun_family = AF_UNIX;
        strncpy(sockaddr.sun_path, a_socketFilename,
                sizeof(sockaddr.sun_path) - 1);

        result = bind(Socketdescriptor, (struct sockaddr *) &sockaddr,
                sizeof(struct sockaddr_un)) ;

        if( -1 == result  )
        {
            TRACE_ERR(" : bind(%d)=%d errn=%d %s ",
                    Socketdescriptor,
                    result,errno,strerror(errno) );
            close(Socketdescriptor);
            result = errno;
        }
        else
        {
            *a_pSocketdescriptor = Socketdescriptor;
        }
    }

    return result;
}

