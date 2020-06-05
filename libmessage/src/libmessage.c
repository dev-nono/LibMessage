/*
 * libmessage.c
 *
 *  Created on: 5 juin 2020
 *      Author: bruno
 */


#include <errno.h>
#include <pthread.h>

// libmessage.h
#include <limits.h> // libmessage.h

#include "libmessage.h"
#include "libmessage_int.h"


struct sdataThread
{
    pthread_t       ThreadID;
    pthread_attr_t  Attr;


};

typedef struct sdataThread dataThread_t;

static dataThread_t gDataThread = {0};

//****************************************************
//*
//*
//****************************************************
int libmessage_manageMessage(const char *a_Message)
{
    int result = 0;

    if( 0 == *a_Message)
    {
        result = EINVAL;
    }



    return result;
}
//****************************************************
//*
//*
//****************************************************
static void * libmessage_threadFunction(void * a_pArg)
{
    (void) a_pArg;

    do
    {
        // wait condition variable
        // si list vide , wait condition variable
        // sinon epoll sur MQ ouvert


        //libmessage_m


    }while(1);


    return 0;
}
//****************************************************
//*
//*
//****************************************************
int libmessage_init()
{
    int result = 0;

    //*****************************
    // create new tread for listening incomming messages
    //*****************************
    result =  pthread_create(   &gDataThread.ThreadID,
                                NULL,
                                &libmessage_threadFunction,
                                0);


    return result;
}
//****************************************************
//*
//*
//****************************************************
int libmessage_close()
{
    int result = 0;


    result = pthread_cancel(gDataThread.ThreadID);


    return result;
}

//****************************************************
//*
//*
//****************************************************
int libmessage_server_register(const char * a_ServiceName)
{
    int result = 0;

    // find struct service

    // add service in list

    // notify thread

    return result;
}
//****************************************************
//*
//*
//****************************************************
int libmessage_server_wait()
{
    int result = 0;

    result = pthread_join(gDataThread.ThreadID,0);

    // wait on end of thread
    return result;
}
//****************************************************
//*
//*
//****************************************************
int libmessage_client_register()
{
    int result = 0;

    return result;
}

