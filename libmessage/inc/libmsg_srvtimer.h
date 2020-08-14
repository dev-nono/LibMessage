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

#if  0
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

#endif
//*****************************************************
struct sTimerRequest
//*****************************************************
{
    struct timespec     timespesc; // timeout for timer
};
typedef struct sTimerRequest  sTimerRequest_t;

//*****************************************************
struct sTimerResponse
//*****************************************************
{
    struct timespec     timespesc;
};
typedef struct sTimerResponse  sTimerResponse_t;



//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//            generic CLIENT / SERVER
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
int libmsg_srvtimer_srv_wait();
int libmsg_srvtimer_cli_wait();

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//          svc getdate
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
/**
 * \fn      int libmsg_srvtimer_cli_getdate(const char* a_Servicename , double *a_Date)
 *
 * \brief   call service "getdate"
 *
 * \param   const char  *a_servername : name of server to call svc getdate
 * \param   double      *a_Date       : output pointer to double
 *
 * \return      0 is ok
 *              or error code
 */
int libmsg_srvtimer_cli_getdate(const  char* a_servername , double *a_pDate);

/**
 * \fn       libmsg_srvtimer_srv_register_svc_getdate(char *a_filenameserver,libmsg_pFunctCB_t a_pFunctCB);
 *
 * \brief       server side, register svc getdate
 *
 * \param   char                *a_filenameserver   :  name of socker svc server
 * \param   libmsg_pFunctCB_t   a_pFunctCB          :  callback to call for this svc
 *
 * \return  0 == OK
 *          error code
 */
int libmsg_srvtimer_srv_register_svc_getdate(char *a_filenameserver,libmsg_pFunctCB_t a_pFunctCB);

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//          svc timer
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

/**
 * \fn       libmsg_srvtimer_cli_timer(
 *          char                        *a_filenameserver,
 *          const double                a_Date,
 *          libmsg_pFunctCB_response_t  a_pFunctCB) // TODO
 * \brief
 *
 * \param
 * \return
 */
int libmsg_srvtimer_cli_timer(  char                        *a_filenameserver,
                                const struct timespec       a_Timeout,
                                libmsg_pFunctCB_response_t  a_pFunctCB,
                                sSignal_t                   *a_pDataSvcTimer );

/**
 * \fn      int libmsg_cli_register_svc_Signal(sThreadDataCtxSignal_t *a_pThreadDataCtx)
 * \brief
 *
 * \param
 * \return
 */
int libmsg_cli_register_svc_Signal(sThreadDataCtxSignal_t *a_pThreadDataCtx);

/**
 * \fn          int libmsg_srvtimer_srv_register_svc_timer(char *a_filenameserver,libmsg_pFunctSignalCB_t a_pFunctCB);
 * \brief
 *
 * \param
 * \return
 */
int libmsg_srvtimer_srv_register_svc_timer(char *a_filenameserver,libmsg_pFunctSignalCB_t a_pFunctCB);


#endif /* INC_LIBMSG_SRVTIMER_H_ */

