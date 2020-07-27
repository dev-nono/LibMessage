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

#define SVC_GETDATE     "getdate"

//******************************************************
//  services "server_time"
//******************************************************
#define SERVER_TIME             "/srvtime"
#define SERVER_TIME_GETDATE     SERVER_TIME"."SVC_GETDATE
#define SERVER_TIME_SETDATE     SERVER_TIME".setdate"
#define SERVER_TIME_SIGNAL      SERVER_TIME".signal"



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

    char filenameClient[PATH_MAX];
//    char data __flexarr;    /* Name.  */
    char        data[PIPE_BUF] ;// __flexarr;    /* Name.  */

};
typedef struct sRequest sRequest_t;

//*****************************************************
struct sResponse
//*****************************************************
{
    sHeader_t   header;
    char        data[PIPE_BUF] ;// __flexarr;    /* Name.  */

};
typedef struct sResponse sResponse_t;
//*****************************************************
struct sDataService
//*****************************************************
{
    char                    filenameServer[NAME_MAX];
    char                    filenameSemaphore[NAME_MAX];

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
    struct pollfd   pollFdClient[MAX_POLL_FD];
    nfds_t          nfds;

};
typedef struct sDataThreadCtx sDataThreadCtx_t;
int     libmsg_cli_getdata(
        _IN_ const char     *a_Srvname,
        _IN_ const char     *a_Clientname,
        _IN_ const uint32_t  a_SizeBuffIn,
        _IN_ const char     *a_BufferIN,
        _IN_ const uint32_t  a_SizeBuffOut,
        _OUT_      char     *a_BufferOUT);





static void * libmsg_server_threadFunction(void * a_pArg);



#endif /* INC_LIBMSG_H_ */
