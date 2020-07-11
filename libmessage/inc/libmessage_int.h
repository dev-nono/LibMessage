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

#include "libmessage.h"



#define SVR_TIME            "srvtime"
#define SVC_GETDATE         "getdate"
#define SVR_TIME_GETDATE_SEM    SVR_TIME"."SVC_GETDATE

//******************************************************
//  services "server_time"
//******************************************************
#define PATH_SRVNAME_TIME           "/tmp/srvtime"
#define FILENAME_SVC_TIME_GETDATE   PATH_SRVNAME_TIME"_getdate"
//#define SVCNAME_TIME_SETDATE     PATH_SRVNAME_TIME"_setdate"
//#define SVCNAME_TIME_SIGNAL      PATH_SRVNAME_TIME"_signal"
//#define SVCNAME_TIME_END              ""


const char* get_arrayServiceName(uint32_t a_ServiceID );

struct sDataService
{
    char            filenameClient[NAME_MAX+(1)];
    char            filenameServer[NAME_MAX+(1)];
    libmessage_pFunctCB_t       pFunctCB;
    int             id;
    sem_t           *pSemsvc;

    char            databuffer[PIPE_BUF];
};
typedef struct sDataService   sDataService_t;


typedef struct pollfd pollfd_t;

struct sDataThreadCtx
{
    pthread_t       pthreadID;
    pthread_attr_t  attr;
    sDataService_t  dataService;

    pid_t           pid;

//    pollfd_t        arrayPollfd[LIBMESSAGE_MAX_ARRAY];
//    sDataService_t  arrayDataService[LIBMESSAGE_MAX_ARRAY];
//    int             nbItem;

};
typedef struct sDataThreadCtx sDataThreadCtx_t;

int libmessage_svc_getdata(sDataService_t *a_pDataService);

int libmessage_mkfifo(const char * a_Fifoname);
int libmessage_openfifo(const char * a_Fifoname,uint32_t a_flag ,int *a_pFd);

void * libmessage_threadFunction_srv(void * a_pArg);



#endif /* INC_LIBMESSAGE_INT_H_ */
