/*
 * libmessage_svc_time.h
 *
 *  Created on: 3 juil. 2020
 *      Author: bruno
 */

#ifndef INC_LIBMESSAGE_SVC_TIME_H_
#define INC_LIBMESSAGE_SVC_TIME_H_

#include <time.h>

#include "libmessage.h"


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



int libmessage_srvtime_wait();

//******************************************************
//  service time part client
//******************************************************
//************************************************************
//  client side
//      return:
//          SUCCESS    0
//          EINVAL          22  Invalid argument
//************************************************************
int libmessage_getdate(
        _IN_  const char *a_Callername,   // address fifo name to respond
        _OUT_ double     *a_pDate);        // buffer data output

int libmessage_setdate(
        _IN_  const char *a_Callername,   // address fifo name to respond
        _IN_ double     a_Date);        // buffer data output

//******************************************************
//  service time part SERVER
//******************************************************

int libmessage_srvtime_register_getdate(libmessage_pFunctCB_t a_pFunctCB);
int libmessage_srvtime_register_setdate(libmessage_pFunctCB_t a_pFunctCB);
int libmessage_srvtime_register_signaldate(libmessage_pFunctCB_t a_pFunctCB);



#endif /* INC_LIBMESSAGE_SVC_TIME_H_ */
