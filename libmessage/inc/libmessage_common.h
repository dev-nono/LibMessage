/*
 * libmessage_common.h
 *
 *  Created on: 11 juin 2020
 *      Author: bruno
 */

#ifndef INC_LIBMESSAGE_COMMON_H_
#define INC_LIBMESSAGE_COMMON_H_

#define SVR_TIME            "srvtime"
#define SVC_GETDATE         "getdate"
#define SVR_TIME_GETDATE    "/tmp/"SVR_TIME"."SVC_GETDATE

#define SVR_TIME_GETDATE_SEM "/sem_"SVR_TIME"_"SVC_GETDATE

typedef int (*pFuncCB_t)(char*);



#endif /* INC_LIBMESSAGE_COMMON_H_ */
