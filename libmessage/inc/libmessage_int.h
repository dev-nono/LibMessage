/*
 * libmessage_int.h
 *
 *  Created on: 5 juin 2020
 *      Author: bruno
 */

#ifndef INC_LIBMESSAGE_INT_H_
#define INC_LIBMESSAGE_INT_H_

// include ==> #include "libmessage_int.h"
#include <mqueue.h>
#include <limits.h>
#include <poll.h>
#include <stdint.h>
#include <semaphore.h>

#include "libmessage_svc_time.h"



enum eLIBMSG_COL
{
    eLIBMSG_COL_PREFIX  = 0,
    eLIBMSG_COL_SEM,
    eLIBMSG_COL_SRV_FILENAME
};
typedef enum eLIBMSG_COL eLIBMSG_COL_t;
//******************************************************
//  services "server_time"
//******************************************************

#define SVR_TIME            "srvtime"

#define SVC_GETDATE         "getdate"
#define SVC_SETDATE         "setdate"
#define SVC_SIGNALDATE      "signaldate"

#define SVC_TIME_GETDATE_PREFIX    SVR_TIME"."SVC_GETDATE
#define SVC_TIME_SETDATE_PREFIX    SVR_TIME"."SVC_SETDATE
#define SVC_TIME_SIGNALDATE_PREFIX SVR_TIME"."SVC_SIGNALDATE

#define SVR_TIME_GETDATE_SEM    SVR_TIME"_"SVC_GETDATE
#define SVR_TIME_SETDATE_SEM    SVR_TIME"_"SVC_SETDATE
#define SVR_TIME_SIGNALDATE_SEM SVR_TIME"_"SVC_SIGNALDATE

#define PATH_SRVNAME_TIME               "/tmp/"SVR_TIME

#define SRV_TIME_GETDATE_FILENAME       PATH_SRVNAME_TIME"_"SVC_GETDATE
#define SRV_TIME_SETDATE_FILENAME       PATH_SRVNAME_TIME"_"SVC_SETDATE
#define SRV_TIME_SIGNALDATE_FILENAME    PATH_SRVNAME_TIME"_"SVC_SIGNALDATE

const char* get_arrayServiceName(uint32_t a_ServiceID );





//*****************************************************
struct sResponse
//*****************************************************
{
    int     result;

    union
    {
        sGetdateResponse_t      getdate;
        sSetdateResponse_t      setdate;
        sSignaldateResponse_t   signaldate;
   }uResponse;
};
typedef struct sResponse sResponse_t;

//*****************************************************
struct sRequest
//*****************************************************
{
    char                    filenameClient[PATH_MAX];

    union
    {
        sGetdateRequest_t      getdata;
        sSetdateRequest_t      setdate;
        sSignaldateRequest_t   signaldate;
   }uRequest;
};
typedef struct sRequest sRequest_t;

//typedef int (*libmessage_pFunctCB_t)(const sRequest_t* , sResponse_t*);

//*****************************************************
struct sDataService
//*****************************************************
{
    char                    filenameServer[NAME_MAX+(1)];

    libmessage_pFunctCB_t   pFunctCB;
    int                     id;
    sem_t                   *pSemsvc;

    char            databuffer[PIPE_BUF];

    sRequest_t     request;
    sResponse_t    response;
};
typedef struct sDataService   sDataService_t;


typedef struct pollfd pollfd_t;

struct sDataThreadCtx
{
    pthread_t       pthreadID;
    pthread_attr_t  attr;
    sDataService_t  dataService;

    pid_t           pid;
};
typedef struct sDataThreadCtx sDataThreadCtx_t;


//**********************************************************
//*             SERVER
//**********************************************************
int libmessage_srvtime_register_svc(    eLIBMSG_ID_t            a_MessageID,
                                        libmessage_pFunctCB_t   a_pFunctCB );

void * libmessage_threadFunction_srv(void * a_pArg);

//**********************************************************
//*             client
//**********************************************************
int libmessage_svc_getdata(sDataService_t *a_pDataService);

//**********************************************************
//*             COMMON
//**********************************************************

int libmessage_mkfifo(const char * a_Fifoname);
int libmessage_openfifo(const char * a_Fifoname,uint32_t a_flag ,int *a_pFd);

sDataThreadCtx_t    *getTheadCtx(eLIBMSG_ID_t a_ID);
const char          *getNameService(eLIBMSG_ID_t a_ID,eLIBMSG_COL_t a_ColID);





#endif /* INC_LIBMESSAGE_INT_H_ */
