/*
 * libmessage_int.h
 *
 *  Created on: 5 juin 2020
 *      Author: bruno
 */

#ifndef INC_LIBMESSAGE_INT_H_
#define INC_LIBMESSAGE_INT_H_

<<<<<<< HEAD
// include ==> #include "libmessage_int.h"
#include <mqueue.h>
#include <limits.h>
#include <poll.h>
#include <stdint.h>

#include "libmessage_common.h"

#define LIBMESSAGE_MAX_BUFFER 1024

//******************************************************
//  services "server_time"
//******************************************************
#define SERVER_TIME             "/server_time"
#define SERVER_TIME_GETDATE     SERVER_TIME".getdate"
#define SERVER_TIME_SETDATE     SERVER_TIME".setdate"
#define SERVER_TIME_SIGNAL      SERVER_TIME".signal"


const char* get_arrayServiceName(uint32_t a_ServiceID );

struct sDataService
{
    char            filenameClient[NAME_MAX+1];
    char            filenameServer[NAME_MAX+1];
    pFuncCB_t       pFuncCB;
    int             id;
};
typedef struct sDataService   sDataService_t;


struct sDataThread
{
    pthread_t       pthreadID;
    pthread_attr_t  Attr;

    pid_t           pid;

    struct pollfd   arrayPollfd[100];
    sDataService_t  arrayDataService[100];
    int             nbItem;
};
typedef struct sDataThread sDataThread_t;


int libmessage_register_serviceID(
        sDataThread_t *a_pContext,
        uint32_t    a_ServiceID ,   // SERVER_TIME_ID_GETDATE, SERVER_TIME_ID_SETDATE SERVER_TIME_ID_xxx
        pFuncCB_t   a_pFuncCB );

//static int libmessage_sendEvent(sDataThread_t *a_pContext, uint32_t a_IndexService );
//int libmessage_server_register_service_time(int         a_ServiceID ,
//                                            pFuncCB_t   a_pFuncCB);


//int libmessage_manageMessage(const char *a_Message);
//int libmessage_pollCheck();

=======
int libmessage_manageMessage(const char *a_Message);
>>>>>>> branch 'master' of https://github.com/dev-nono/LibMessage.git

#endif /* INC_LIBMESSAGE_INT_H_ */
