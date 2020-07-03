/*
 * libmessage.h
 *
 *  Created on: 4 juin 2020
 *      Author: bruno
 */

#ifndef INC_LIBMESSAGE_H_
#define INC_LIBMESSAGE_H_

#include <stdint.h>
#include <semaphore.h>

#include "libmessage_common.h"



struct sData_Getdate
{
    char endpointResponse[NAME_MAX];

};


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
#define LIBMESSAGE_SRVID_TIME       (0U)
#define LIBMESSAGE_SRVID_NETWORK    (1U)
#define LIBMESSAGE_SRVID_END        (LIBMESSAGE_SRVID_NETWORK+(1U))

//*******************************************************
//          SERVICES ID
//*******************************************************
#define LIBMESSAGE_SVCID_TIME_GETDATE  (0U)
#define LIBMESSAGE_SVCID_TIME_SETDATE  (1U)
#define LIBMESSAGE_SVCID_TIME_SIGNAL   (2U)
#define LIBMESSAGE_SVCID_TIME_END      (LIBMESSAGE_SVCID_TIME_SIGNAL+(1U))


#define LIBMESSAGE_SVCID_NET_ADD       (0U)
#define LIBMESSAGE_SVCID_NET_REMOVE    (1U)
#define LIBMESSAGE_SVCID_NET_END       (2U)

int libmessage_getdate( const char *a_Callername,
                        //const char *a_Servername,
        uint32_t         a_ServiceID,
                        double     *a_Date,
                        sem_t *a_pSemGedate);


//******************************************************
//  service time
//******************************************************
int libmessage_srvtime_init();

//******************************************************
//
//******************************************************
int libmessage_register_service(
        uint32_t    a_ServerID ,   // LIBMESSAGE_SRVID_TIME  LIBMESSAGE_SRVID_NETWORK
        uint32_t    a_ServiceID , // SERVER_TIME_ID_GETDATE, SERVER_TIME_ID_SETDATE SERVER_TIME_ID_xxx
        pFuncCB_t   a_pFuncCB);

//******************************************************
//  generic function
//******************************************************
//int libmessage_init();
//int libmessage_close();

const char * getStrDate();
int libmessage_server_wait();
int libmessage_mkfifo(const char * a_Fifoname);
int libmessage_openfifo(const char * a_Fifoname,uint32_t a_flag ,int *a_pFd);

//int libmessage_client_register();


#endif /* INC_LIBMESSAGE_H_ */
