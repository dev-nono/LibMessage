/*
 * libmessage_svc_time.h
 *
 *  Created on: 3 juil. 2020
 *      Author: bruno
 */

#ifndef INC_LIBMESSAGE_SVC_TIME_H_
#define INC_LIBMESSAGE_SVC_TIME_H_

#include "libmessage.h"

int libmessage_srvtime_wait();

//******************************************************
//  service time part client
//******************************************************
int libmessage_getdate(
        _IN_  const char *a_Callername,   // address fifo name to respond
        _OUT_ double     *a_Date);        // buffer data output

//******************************************************
//  service time part SERVER
//******************************************************

int libmessage_srvtime_register_getdate(libmessage_pFunctCB_t a_pFunctCB);
int libmessage_srvtime_register_setdate(libmessage_pFunctCB_t a_pFunctCB);
int libmessage_srvtime_register_signaldate(libmessage_pFunctCB_t a_pFunctCB);



#endif /* INC_LIBMESSAGE_SVC_TIME_H_ */
