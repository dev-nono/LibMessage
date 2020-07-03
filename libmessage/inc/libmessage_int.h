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

#include "libmessage_common.h"

#define LIBMESSAGE_MAX_BUFFER   (1024U)
#define LIBMESSAGE_MAX_ARRAY    (100U)

//******************************************************
//  services "server_time"
//******************************************************
#define SRVNAME_TIME             "/srv_time"
#define SVCNAME_TIME_GETDATE     SRVNAME_TIME".getdate"
#define SVCNAME_TIME_SETDATE     SRVNAME_TIME".setdate"
#define SVCNAME_TIME_SIGNAL      SRVNAME_TIME".signal"
#define SVCNAME_TIME_END              ""

const char* get_arrayServiceName(uint32_t a_ServiceID );

struct sDataService
{
    char            filenameClient[NAME_MAX+(1)];
    char            filenameServer[NAME_MAX+(1)];
    pFuncCB_t       pFuncCB;
    int             id;
    sem_t           *pSemsvc;

    char            databuffer[PIPE_BUF];
};
typedef struct sDataService   sDataService_t;


typedef struct pollfd pollfd_t;

struct sDataThreadCtx
{
    pthread_t       pthreadID;
    pthread_attr_t  Attr;

    pid_t           pid;

    pollfd_t        arrayPollfd[LIBMESSAGE_MAX_ARRAY];
    sDataService_t  arrayDataService[LIBMESSAGE_MAX_ARRAY];
    int             nbItem;
};
typedef struct sDataThreadCtx sDataThreadCtx_t;



int libmessage_mkfifo(const char * a_Fifoname);
int libmessage_openfifo(const char * a_Fifoname,uint32_t a_flag ,int *a_pFd);

//int libmessage_register_serviceID(
//        sDataThreadCtx_t *a_pContext,
//        uint32_t    a_ServiceID ,   // SERVER_TIME_ID_GETDATE, SERVER_TIME_ID_SETDATE SERVER_TIME_ID_xxx
//        pFuncCB_t   a_pFuncCB );

//static int libmessage_sendEvent(sDataThread_t *a_pContext, uint32_t a_IndexService );
//int libmessage_server_register_service_time(int         a_ServiceID ,
//                                            pFuncCB_t   a_pFuncCB);


//int libmessage_manageMessage(const char *a_Message);
//int libmessage_pollCheck();



////******************************************************
////  services "server_time"
////******************************************************
//#define SERVER_TIME             "/server_time"
//#define SERVER_TIME_GETDATE     SERVER_TIME".getdate"
//#define SERVER_TIME_SETDATE     SERVER_TIME".setdate"
//#define SERVER_TIME_SIGNAL      SERVER_TIME".signal"

//*******************************************************
//          SERVER ID
//*******************************************************
//#define LIBMESSAGE_SRVID_TIME       (0U)
//#define LIBMESSAGE_SRVID_NETWORK    (1U)
//#define LIBMESSAGE_SRVID_END        (LIBMESSAGE_SRVID_NETWORK+(1U))
//
////*******************************************************
////          SERVICES ID
////*******************************************************
//#define LIBMESSAGE_SVCID_TIME_GETDATE  (0U)
//#define LIBMESSAGE_SVCID_TIME_SETDATE  (1U)
//#define LIBMESSAGE_SVCID_TIME_SIGNAL   (2U)
//#define LIBMESSAGE_SVCID_TIME_END      (LIBMESSAGE_SVCID_TIME_SIGNAL+(1U))
//
//
//#define LIBMESSAGE_SVCID_NET_ADD       (0U)
//#define LIBMESSAGE_SVCID_NET_REMOVE    (1U)
//#define LIBMESSAGE_SVCID_NET_END       (2U)

////******************************************************
////
////******************************************************
//int libmessage_register_service(
//        uint32_t    a_ServerID ,   // LIBMESSAGE_SRVID_TIME  LIBMESSAGE_SRVID_NETWORK
//        uint32_t    a_ServiceID , // SERVER_TIME_ID_GETDATE, SERVER_TIME_ID_SETDATE SERVER_TIME_ID_xxx
//        pFuncCB_t   a_pFuncCB);


#endif /* INC_LIBMESSAGE_INT_H_ */
