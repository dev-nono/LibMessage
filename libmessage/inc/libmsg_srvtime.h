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
//                  SERVER
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
int libmsg_srvtime_srv_wait();



//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//                  CLIENT
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
int libmsg_srvtime_cli_wait();

//****************************************************
//          svc getdate
//****************************************************
int libmsg_srvtime_cli_getdate(double *a_pDate);

int libmsg_srvtime_srv_register_svc_getdate(libmsg_pFunctCB_t a_pFunctCB);

//****************************************************
//          svc signaldate
//****************************************************
int libmsg_srvtime_cli_signaldate(  const char* a_UniqID,
                                    const double a_Date,
                                    libmsg_pFunctCB_t a_pFunctCB);

int libmsg_srvtime_srv_register_svc_signal(libmsg_pFunctCB_t a_pFunctCB);

int libmsg_srvtime_register_signal(sDataThreadCtx_t *a_pDataThreadCtx);
int libmsg_srvtime_srv_signaldate_addClientNotify(const sRequest_t *a_pRequest);

#endif /* INC_LIBMSG_SRVTIME_H_ */

