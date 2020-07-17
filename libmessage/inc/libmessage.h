/*
 * libmessage.h
 *
 *  Created on: 4 juin 2020
 *      Author: bruno
 */

#ifndef INC_LIBMESSAGE_H_
#define INC_LIBMESSAGE_H_

#include "limits.h"
#include <semaphore.h>


#define _IN_
#define _OUT_
#define _INOUT_

typedef int (*libmessage_pFunctCB_t)(const void * , void *);


enum eLIBMSG_COL
{
    eLIBMSG_COL_PREFIX  = 0,
    eLIBMSG_COL_SEM,
    eLIBMSG_COL_SRV_FILENAME
};
typedef enum eLIBMSG_COL eLIBMSG_COL_t;


//*****************************************************
struct sRequest
//*****************************************************
{
    char        filenameClient[PATH_MAX];
    int         result;
    uint32_t    datasize;

    char data __flexarr;    /* Name.  */

};
typedef struct sRequest sRequest_t;

//*****************************************************
struct sResponse
//*****************************************************
{
    int         result;
    uint32_t    datasize;

    char data __flexarr;    /* Name.  */

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

    char            databuffer[PIPE_BUF];

    sRequest_t     request;
    sResponse_t    response;
};
typedef struct sDataService   sDataService_t;


typedef struct pollfd pollfd_t;

struct sThreadCtx
{
    pthread_t       pthreadID;
    pthread_attr_t  attr;
    sem_t           semsvc;

};
typedef struct sThreadCtx sThreadCtx_t;

struct sDataThreadCtx
{
    pthread_t       pthreadID;
    pthread_attr_t  attr;

    sDataService_t  dataService;

};
typedef struct sDataThreadCtx sDataThreadCtx_t;



//******************************************************
//  generic function
//******************************************************

#endif /* INC_LIBMESSAGE_H_ */
