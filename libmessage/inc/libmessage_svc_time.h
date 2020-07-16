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

typedef int (*libmessage_pFunctSignalCB_t)(const struct timespec);


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



//******************************************************
//*
//*             service time part client
//*
//******************************************************
//************************************************************
//  client side
//      return:
//          SUCCESS    0
//          EINVAL          22  Invalid argument
//          errors ...
//************************************************************
int libmessage_getdate(_OUT_ double *a_pDate);        // buffer data output

//************************************************************
//  client side
//      IN_ double a_Date   : buffer data output
//      return:
//          SUCCESS    0
//          EINVAL          22  Invalid argument
//          errors ...
//************************************************************
int libmessage_setdate(_IN_ double a_Date); // buffer data output
//************************************************************
//  client side
//      IN_ double a_Date   : buffer data output
//      return:
//          SUCCESS    0
//          EINVAL          22  Invalid argument
//          errors ...
//************************************************************
int libmessage_signaldate(
        _IN_    double     a_Date,         // buffer data output
        _IN_    libmessage_pFunctSignalCB_t a_pFunct);// callback

//******************************************************
//*
//*
//*          service time part SERVER
//*
//*
//******************************************************

int libmessage_srvtime_register_getdate(libmessage_pFunctCB_t a_pFunctCB);
int libmessage_srvtime_register_setdate(libmessage_pFunctCB_t a_pFunctCB);
int libmessage_srvtime_register_signaldate(libmessage_pFunctCB_t a_pFunctCB);

int libmessage_srvtime_wait();


#endif /* INC_LIBMESSAGE_SVC_TIME_H_ */
