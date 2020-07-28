/*
 * libmsg_srvtime.h
 *
 *  Created on: 27 juil. 2020
 *      Author: bruno
 */

#ifndef INC_LIBMSG_SRVTIME_H_
#define INC_LIBMSG_SRVTIME_H_

#include "libmessage_common.h"
#include "libmsg.h"

int libmsg_srvtime_getdate(const char* a_UniqID,double *a_pDate);

int libmsg_srvtime_register_getdate(libmsg_pFunctCB_t a_pFunctCB);
int libmsg_srvtime_wait();



#endif /* INC_LIBMSG_SRVTIME_H_ */
