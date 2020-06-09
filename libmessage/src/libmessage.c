/*
 * libmessage.c
 *
 *  Created on: 5 juin 2020
 *      Author: bruno
 */


#include <errno.h>
#include <pthread.h>
#include <poll.h>


// include ==> #include libmessage_int.h
#include <mqueue.h>
#include <limits.h>
#include "libmessage_int.h"


// include ==> libmessage.h
//#include <limits.h>
#include "libmessage.h"



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
    int             result  = 0;
    struct pollfd   arrayPollfd[3]  = {0};
    nfds_t          nfds    = 0;

    (void) a_pArg;

    do
    {
        // wait condition variable
        // si list vide , wait condition variable
        // sinon epoll sur MQ ouvert


        //libmessage_m
        result = poll(arrayPollfd, nfds, -1);
        if ( result > 0 )
        {
            result = libmessage_pollCheck();
        }
        else if (0 == result )
        {
            // timeout
        }
        else // error
        {
            // print error

        }


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
//****************************************************
//*
//*
//****************************************************
int libmessage_pollCheck()
{
    int result = 0;

    return result;
}


