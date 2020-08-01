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


#include "libmessage_common.h"
#include "utils.h"

#define MAX_POLL_FD (100U)


// client_1: cli.1.getdate
// server  : srvtime.getdate
//
// client_1: cli.1.signal1
// server  : srvtime.signal

#define SVC_GETDATE         "getdate"
#define SVC_SIGNALDATE      "signaldate"
//******************************************************
//  services "server_time"
//******************************************************
#define SERVER_TIME             "/srvtime"
#define SERVER_TIME_GETDATE     SERVER_TIME"."SVC_GETDATE
#define SERVER_TIME_SETDATE     SERVER_TIME".setdate"
#define SERVER_TIME_SIGNALDATE  SERVER_TIME".signal"

#define DATA_MAX_REQUEST    (HARD_MAX - NAME_MAX - sizeof(sHeader_t))
#define DATA_MAX_RESPONSE    (HARD_MAX - sizeof(sHeader_t))


//*****************************************************
struct sHeader
//*****************************************************
{
    int         result;
    uint32_t    datasize;
};
typedef struct sHeader sHeader_t;

//*****************************************************
struct sRequest
//*****************************************************
{
    sHeader_t   header;

    char filenameClient[NAME_MAX];
//    char data __flexarr;    /* Name.  */
    char        data[DATA_MAX_REQUEST] ;// __flexarr;    /* Name.  */

};
typedef struct sRequest sRequest_t;

//*****************************************************
struct sResponse
//*****************************************************
{
    sHeader_t   header;
    char        data[DATA_MAX_RESPONSE] ;// __flexarr;    /* Name.  */

};
typedef struct sResponse sResponse_t;

typedef int (*libmsg_pFunctCB_t)(const sRequest_t  *a_pRequest,sResponse_t *a_pResponse);

//*****************************************************
struct sDataService
//*****************************************************
{
    char                    filenameServer[NAME_MAX];
    //char                    filenameSemaphore[NAME_MAX];

    libmsg_pFunctCB_t   pFunctCB;
    int                     id;

    sRequest_t     request;
    sResponse_t    response;


};
typedef struct sDataService   sDataService_t;


struct sDataThreadCtx
{
    pthread_t       pthreadID;
    pthread_attr_t  attr;

    sDataService_t  dataService;

    // for client signal;
    sRequest_t      request[MAX_POLL_FD];
    int             filedescriptor[MAX_POLL_FD];
    //struct pollfd   pollFdClient[MAX_POLL_FD];
    nfds_t          nfds;

};
typedef struct sDataThreadCtx sDataThreadCtx_t;


int     libmsg_cli_getdata(
        _IN_ const char     *a_Srvname,
        sDataService_t      *a_pDataService);
//        const sRequest_t    *a_pRequest,
//        sResponse_t         *a_pResponse);

int libmsg_srv_register_svc(sDataThreadCtx_t *a_pDataThreadCtx);
int libmsg_cli_register_svc(sDataThreadCtx_t *a_pDataThreadCtx);

int libmsg_srv_find_registred_client(
        sRequest_t      *a_pRequest,
        unsigned int    a_nfds,
        const char      *a_filenameClient);




#endif /* INC_LIBMSG_H_ */
