/*
 * libmsg_srvtime.h
 *
 *  Created on: 27 juil. 2020
 *      Author: bruno
 */

#ifndef INC_LIBMSG_SRVTIMER_H_
#define INC_LIBMSG_SRVTIMER_H_

#include "libmessage_common.h"
#include "libmsg.h"


//*****************************************************
struct sGetdateRequest
//*****************************************************
{
    //struct timespec timespesc;
};
typedef struct sGetdateRequest  sGetdateRequest_t;

//*****************************************************
struct sGetdateResponse
//*****************************************************
{
    struct timespec timespesc;
};
typedef struct sGetdateResponse  sGetdateResponse_t;

//*****************************************************
struct sSetdateRequest
//*****************************************************
{
    struct timespec     timespesc;
};
typedef struct sSetdateRequest sSetdateRequest_t;

//*****************************************************
struct sSetdateResponse
//*****************************************************
{
//    struct timespec     timespesc;
};
typedef struct sSetdateResponse sSetdateResponse_t;

//*****************************************************
struct sSignaldateRequest
//*****************************************************
{
    struct timespec     timespesc; // timeout for timer
};
typedef struct sSignaldateRequest  sSignaldateRequest_t;

//*****************************************************
struct sSignaldateResponse
//*****************************************************
{
    struct timespec     timespesc;
};
typedef struct sSignaldateResponse  sSignaldateResponse_t;



//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//            generic CLIENT / SERVER
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
int libmsg_srvtimer_srv_wait();
int libmsg_srvtimer_cli_wait();

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//          svc getdate
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
int libmsg_srvtimer_cli_getdate(double *a_pDate);

int libmsg_srvtimer_srv_register_svc_getdate(libmsg_pFunctCB_t a_pFunctCB);

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//          svc timer
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

int libmsg_srvtimer_cli_timer(const double a_Date,libmsg_pFunctCB_response_t a_pFunctCB);

int libmsg_srvtimer_srv_register_svc_timer(libmsg_pFunctSignalCB_t a_pFunctCB);


#endif /* INC_LIBMSG_SRVTIMER_H_ */

