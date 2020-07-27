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

#endif /* INC_LIBMSG_SRVTIME_H_ */
