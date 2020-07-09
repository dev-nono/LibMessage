/*
 * libmessage.h
 *
 *  Created on: 4 juin 2020
 *      Author: bruno
 */

#ifndef INC_LIBMESSAGE_H_
#define INC_LIBMESSAGE_H_

#include <stdint.h>


void * libmessage_threadFunction_srv(void * a_pArg);



//******************************************************
//  service time server
//******************************************************

//******************************************************
//  generic function
//******************************************************
const char * getStrDate();

int libmessage_server_wait();

//int libmessage_client_register();


#endif /* INC_LIBMESSAGE_H_ */
