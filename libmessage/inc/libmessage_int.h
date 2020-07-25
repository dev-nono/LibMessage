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

//#include "libmessage_svc_time.h"
#include "libmessage.h"



//******************************************************
//  services "server_time"
//******************************************************

#define SVR_TIME            "srvtime"
#define SVR_SYSTEM          "srvsys"

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





////*****************************************************
//struct sResponse
////*****************************************************
//{
//    int         result;
//    uint32_t    datasize;
//
//    char data __flexarr;    /* Name.  */
//
////    union
////    {
////        sGetdateResponse_t      getdate;
////        sSetdateResponse_t      setdate;
////        sSignaldateResponse_t   signaldate;
////   }uResponse;
//};
//typedef struct sResponse sResponse_t;

////*****************************************************
//struct sRequest
////*****************************************************
//{
//    char        filenameClient[PATH_MAX];
//    int         result;
//    uint32_t    datasize;
//
//    char data __flexarr;    /* Name.  */
//
////    union
////    {
////        sGetdateRequest_t      getdata;
////        sSetdateRequest_t      setdate;
////        sSignaldateRequest_t   signaldate;
////   }uRequest;
//};
//typedef struct sRequest sRequest_t;


//typedef int (*libmessage_pFunctCB_t)(const sRequest_t* , sResponse_t*);




//**********************************************************
//*             SERVER
//**********************************************************

//int             libmessage_server_initialize();

static void *   libmessage_server_threadFunction(void * a_pArg); // TODO
int             libmessage_server_register_svc( sDataThreadCtx_t *a_pDataThreadCtx);
int             libmessage_server_register_fifosignal( sDataThreadCtx_t *a_pDataThreadCtx);

//**********************************************************
//*             client
//**********************************************************

static void *   libmessage_client_threadFunction_signal(void * a_pArg); // TODO

int             libmessage_client_getdataFromServer (sDataService_t *a_pDataService);
int             libmessage_client_register_signal   (sDataService_t *a_pDataService_t);
int             libmessage_client_createThreadSignal(sDataThreadCtx_t *a_pThreadCtx);

//**********************************************************
//*             COMMON
//**********************************************************

int libmessage_mkfifo(const char * a_Fifoname);
int libmessage_openfifo(const char * a_Fifoname,uint32_t a_flag ,int *a_pFd);
int libmessage_openfifo_block(const char * a_Fifoname,uint32_t a_flag ,int *a_pFd);
int libmessage_waitSemaphoreAtDate(sem_t *a_pSemaphore , long long unsigned a_nano);
int libmessage_writeRequest(sDataService_t *a_pDataService,int a_fileDescriptor);
int libmessage_pollResponse(int a_fileDescriptor);
int libmessage_readDataResponse(sDataService_t *a_pDataService, int a_fileDescriptor);








#endif /* INC_LIBMESSAGE_INT_H_ */
