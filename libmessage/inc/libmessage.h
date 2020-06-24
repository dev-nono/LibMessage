/*
 * libmessage.h
 *
 *  Created on: 4 juin 2020
 *      Author: bruno
 */

#ifndef INC_LIBMESSAGE_H_
#define INC_LIBMESSAGE_H_

#include <stdint.h>
#include "libmessage_common.h"


////******************************************************
////  services "server_time"
////******************************************************
//#define SERVER_TIME             "/server_time"
//#define SERVER_TIME_GETDATE     SERVER_TIME".getdate"
//#define SERVER_TIME_SETDATE     SERVER_TIME".setdate"
//#define SERVER_TIME_SIGNAL      SERVER_TIME".signal"

#define LIBMESSAGE_SRVID_TIME       (0U)
#define LIBMESSAGE_SRVID_NETWORK    (1U)
#define LIBMESSAGE_SRVID_END    (LIBMESSAGE_SRVID_NETWORK+(1U))

#define SERVER_TIME_ID_GETDATE  (0U)
#define SERVER_TIME_ID_SETDATE  (1U)
#define SERVER_TIME_ID_SIGNAL   (2U)

#define SERVER_NET_ID_NETADD       (0U)
#define SERVER_NET_ID_NETREMOVE    (1U)

int libmessage_getdate( const char *a_Callername,
                        //const char *a_Servername,
        uint32_t         a_ServiceID,
                        double     *a_Date);

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


int libmessage_server_wait();

//int libmessage_client_register();


#endif /* INC_LIBMESSAGE_H_ */
