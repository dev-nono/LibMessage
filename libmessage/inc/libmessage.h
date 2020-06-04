/*
 * libmessage.h
 *
 *  Created on: 4 juin 2020
 *      Author: bruno
 */

#ifndef INC_LIBMESSAGE_H_
#define INC_LIBMESSAGE_H_


#define SERVER_TIME             "/server_time"
#define SERVER_TIME_GETDATE     SERVER_TIME".getdate"
#define SERVER_TIME_SETDATE     SERVER_TIME".setdate"



int server_time_getdate(const char* a_Caller, double *a_Date);



#endif /* INC_LIBMESSAGE_H_ */
