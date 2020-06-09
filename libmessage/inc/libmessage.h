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


//******************************************************
//  services "server_time"
//******************************************************
#define SERVER_TIME             "/server_time"
#define SERVER_TIME_GETDATE     SERVER_TIME".getdate"
#define SERVER_TIME_SETDATE     SERVER_TIME".setdate"
#define SERVER_TIME_SIGNAL      SERVER_TIME".signal"

int libmessage_getdate( const char *a_Callername,
                        const char *a_Servername,
                        double     *a_Date);


//******************************************************
//  services "server_system"
//******************************************************
int libmessage_server_time_register();

//******************************************************
//  generic function
//******************************************************
int libmessage_init();
int libmessage_close();

int libmessage_server_register(const char * a_ServiceName);
int libmessage_server_wait();

int libmessage_client_register();


#endif /* INC_LIBMESSAGE_H_ */
