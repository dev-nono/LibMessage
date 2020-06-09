/*
 * libmessage_int.h
 *
 *  Created on: 5 juin 2020
 *      Author: bruno
 */

#ifndef INC_LIBMESSAGE_INT_H_
#define INC_LIBMESSAGE_INT_H_

// include ==> #include "libmessage_int.h"
// #include <mqueue.h>
// #include <limits.h>


struct sdataThread
{
    pthread_t       ThreadID;
    pthread_attr_t  Attr;

    pid_t           pid;
};
typedef struct sdataThread dataThread_t;


struct sDataService
{
    mqd_t           fdPidClient;
    char            filenameClient[NAME_MAX+1];
    mqd_t           fdPidServer;
    char            filenameServer[NAME_MAX+1];
};

typedef struct sDataService   sDataService_t;

int libmessage_manageMessage(const char *a_Message);
int libmessage_pollCheck();
#endif /* INC_LIBMESSAGE_INT_H_ */
