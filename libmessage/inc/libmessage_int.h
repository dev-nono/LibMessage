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

#include "libmessage_common.h"

#define LIBMESSAGE_MAX_BUFFER   (1024U)
#define LIBMESSAGE_MAX_ARRAY    (100U)

//******************************************************
//  services "server_time"
//******************************************************
#define SVCNAME_TIME             "/srv_time"
#define SVCNAME_TIME_GETDATE     SVCNAME_TIME".getdate"
#define SVCNAME_TIME_SETDATE     SVCNAME_TIME".setdate"
#define SVCNAME_TIME_SIGNAL      SVCNAME_TIME".signal"
#define SVCNAME_TIME_END              ""

const char* get_arrayServiceName(uint32_t a_ServiceID );

struct sDataService
{
    char            filenameClient[NAME_MAX+(1)];
    char            filenameServer[NAME_MAX+(1)];
    pFuncCB_t       pFuncCB;
    int             id;
    sem_t           *pSemsvc;
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


int libmessage_register_serviceID(
        sDataThreadCtx_t *a_pContext,
        uint32_t    a_ServiceID ,   // SERVER_TIME_ID_GETDATE, SERVER_TIME_ID_SETDATE SERVER_TIME_ID_xxx
        pFuncCB_t   a_pFuncCB );

//static int libmessage_sendEvent(sDataThread_t *a_pContext, uint32_t a_IndexService );
//int libmessage_server_register_service_time(int         a_ServiceID ,
//                                            pFuncCB_t   a_pFuncCB);


//int libmessage_manageMessage(const char *a_Message);
//int libmessage_pollCheck();


int libmessage_srvtime_register_getdate();




#endif /* INC_LIBMESSAGE_INT_H_ */
