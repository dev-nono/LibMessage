/*
 * libmessage.h
 *
 *  Created on: 4 juin 2020
 *      Author: bruno
 */

#ifndef INC_LIBMESSAGE_H_
#define INC_LIBMESSAGE_H_

#include <limits.h>
#include <semaphore.h>
#include <poll.h>

#define MAX_POLL_FD (100U)

#define _IN_
#define _OUT_
#define _INOUT_


//typedef int (*libmessage_pFunctCB_t)(const void * , void *);
typedef int (*libmessage_pFunctCB_t)(void *a_pContext);


enum eLIBMSG_COL
{
    eLIBMSG_COL_PREFIX  = 0,
    eLIBMSG_COL_SEM,
    eLIBMSG_COL_SRV_FILENAME
};
typedef enum eLIBMSG_COL eLIBMSG_COL_t;

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

    libmessage_pFunctCB_t   pFunctCB;
    int                     id;
    sem_t                   *pSemsvc;

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


//******************************************************
//  generic function
//******************************************************


#endif /* INC_LIBMESSAGE_H_ */
