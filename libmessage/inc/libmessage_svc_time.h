/*
 * libmessage_svc_time.h
 *
 *  Created on: 3 juil. 2020
 *      Author: bruno
 */

#ifndef INC_LIBMESSAGE_SVC_TIME_H_
#define INC_LIBMESSAGE_SVC_TIME_H_

#include "libmessage_common.h"

    int libmessage_srvtime_wait();

    //******************************************************
    //  service time part client
    //******************************************************
    int libmessage_getdate( const char *a_Callername,
            double     *a_Date);

    //******************************************************
    //  service time part SERVER
    //******************************************************

    int libmessage_srvtime_register_getdate(pFunctCB_t a_pFunctCB);
    int libmessage_srvtime_register_setdate(pFunctCB_t a_pFunctCB);
    int libmessage_srvtime_register_signaldate(pFunctCB_t a_pFunctCB);



#endif /* INC_LIBMESSAGE_SVC_TIME_H_ */
