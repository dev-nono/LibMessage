/*
 * libmessage.h
 *
 *  Created on: 4 juin 2020
 *      Author: bruno
 */

#ifndef INC_LIBMESSAGE_H_
#define INC_LIBMESSAGE_H_


// libmessage.h
// #include <limits.h> // libmessage.h

typedef int (*pfunction)(const char*);

struct aaaa
{
    char        service_name[NAME_MAX+1];
    pfunction   pFuction;
};



#define SERVER_TIME             "/server_time"
#define SERVER_TIME_GETDATE     SERVER_TIME".getdate"
#define SERVER_TIME_SETDATE     SERVER_TIME".setdate"

int libmessage_register_service();

int server_time_getdate(const char* a_Caller, double *a_Date);



#endif /* INC_LIBMESSAGE_H_ */
