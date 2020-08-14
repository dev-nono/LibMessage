/*
 * libmsg.h
 *
 *  Created on: 27 juil. 2020
 *      Author: bruno
 */

#ifndef INC_LIBMSG_H_
#define INC_LIBMSG_H_

#include <stdint.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>


/**
 * \fn
 * \brief
 *
 * \param
 * \return
 */

#define HARD_MAX  (8192U)


#include "utilstools.h"

#define MAX_POLL_FD (100U)


#define SVC_GETDATE     "getdate"
#define SVC_TIMER       "timer"
//******************************************************
//  services "server_time"
//******************************************************
#define SRVTIMER             "/tmp/srvtimer"
#define SRVTIMER_GETDATE     SRVTIMER"."SVC_GETDATE
#define SRVTIMER_SETDATE     SRVTIMER".setdate"
#define SRVTIMER_TIMER       SRVTIMER".timer"

#define DATA_MAX_REQUEST    (HARD_MAX - NAME_MAX - sizeof(sHeader_t))
#define DATA_MAX_RESPONSE    (HARD_MAX - sizeof(sHeader_t))

struct sSignal
{
    char        filenameClient[NAME_MAX];
    int         socket;
};
typedef struct sSignal sSignal_t;
//*****************************************************
struct sHeader
//*****************************************************
{
    int         result;
    uint32_t    datasize;
};
typedef struct sHeader sHeader_t;

//*****************************************************
struct sRequestClient
//*****************************************************
{
    sHeader_t   header;
    char        filenameClient[NAME_MAX];

    //    char data __flexarr;    /* Name.  */
    char        data[DATA_MAX_REQUEST] ;// __flexarr;    /* Name.  */

};
typedef struct sRequestClient sRequestClient_t;

//*****************************************************
struct sRequestServer
//*****************************************************
{
    sRequestClient_t    request;

    struct sockaddr_storage peer_addr; // NI_NUMERICHOST | NI_NUMERICSERV
    socklen_t               peer_addr_len;

};
typedef struct sRequestServer sRequestServer_t;


//*****************************************************
struct sResponse
//*****************************************************
{
    sHeader_t   header;
    char        data[DATA_MAX_RESPONSE] ;// __flexarr;    /* Name.  */

};
typedef struct sResponse sResponse_t;

typedef int (*libmsg_pFunctCB_t)(       const sRequestServer_t  *a_pRequest,        sResponse_t *a_pResponse);
typedef int (*libmsg_pFunctSignalCB_t)( const sRequestServer_t  *a_pRequestSignal,  sResponse_t *a_pResponse);
typedef int (*libmsg_pFunctCB_response_t)(sResponse_t *a_pResponse);

//*****************************************************
struct sDataService
//*****************************************************
{
    char                filenameServer[NAME_MAX];

    libmsg_pFunctCB_t   pFunctCB;

    sSignal_t           dataSignal;

    sRequestClient_t    request;
    sResponse_t         response;
};
typedef struct sDataService   sDataService_t;


struct sThreadDataCtx
{
    pthread_t       pthreadID;
    pthread_attr_t  attr;
    sDataService_t  dataService;
};
typedef struct sThreadDataCtx sThreadDataCtx_t;

//*****************************************************
struct sDataServiceSignal
//*****************************************************
{
    char                    filenameServer[NAME_MAX];

    sSignal_t           dataSignal;
    libmsg_pFunctCB_t   pFunctCB;
    libmsg_pFunctCB_response_t  pFunctCBresponse;
    int                     id;

    sRequestServer_t    request;
    sResponse_t         response;
};
typedef struct sDataServiceSignal   sDataServiceSignal_t;

struct sThreadDataCtxSignal
{
    pthread_t       pthreadID;
    pthread_attr_t  attr;

    sDataServiceSignal_t  dataService;
};
typedef struct sThreadDataCtxSignal sThreadDataCtxSignal_t;

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
int libmsg_cli_getdata(sDataService_t *a_pDataService, int a_unlinkFilenameclient);


/**
 * \fn      int libmsg_srv_register_svc(sThreadDataCtx_t *a_pThreadDataCtx)
 * \brief   server side create thread dedicated for receive and send data from client
 *          when client use libmsg_cli_getdata(...) for call service
 *
 * \param       sThreadDataCtx_t *a_pThreadDataCtx      : thread data context for one service
 *                  - fill dataService.pFunctCB with static method for custom service
 *                  - fill dataService.filenameServer  name of socket for server service
 *                    used by client to connect
 *
  * \return       error code  of pthread_create()
 */
int libmsg_srv_register_svc(sThreadDataCtx_t *a_pDataThreadCtx);



int libmsg_srv_register_svc_Signal(sThreadDataCtxSignal_t *a_pDataThreadCtx);

/**
 * \fn      sRequestServer_t * libsmg_dupRequestServer(sRequestServer_t *a_pRequest)
 *
 * \brief   duplicate sRequestServer_t  with calloc()
 *
 * \param   sRequestServer_t *a_pRequest : data to dulicate
 *
 * \return  pointer to data dulicated
 */
sRequestServer_t * libsmg_dupRequestServer(sRequestServer_t *a_pRequest);



#endif /* INC_LIBMSG_H_ */
