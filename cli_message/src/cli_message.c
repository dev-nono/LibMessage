/*
 ============================================================================
 Name        : cli_message.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <poll.h>

// include ==>  libmessage.h
#include "libmessage.h"
#include "libmessage_int.h"


int test_msgQueue()
{
    int result = 0;
    int vLenReceive = 0;
    char vClientfilename[PATH_MAX] = "";
    char vServerfilename[PATH_MAX] = SERVER_TIME;
    struct mq_attr  vAttr   = {0};

    struct pollfd fd_client = {0};
    struct pollfd fd_server = {0};

    char    vBuffer[LIBMESSAGE_MAX_BUFFER] = {0};

    // open mq server
    fd_server.fd = mq_open(SERVER_TIME,
            O_WRONLY,S_IRWXO | S_IRWXG | S_IRWXU ,&vAttr);


    // open mq client
    fd_client.fd = mq_open(SERVER_TIME,
            O_WRONLY,S_IRWXO | S_IRWXG | S_IRWXU ,&vAttr);

    // prepare msg

    do{
        // send msg to server


        //receive msg in client
        vLenReceive =  mq_receive(fd_server.fd,
                vBuffer,
                sizeof(vBuffer),
                0U);

    }while(1);


}

int main(void)
{
    int     result = 0;
    double  vDate = 0.0;

//    result = libmessage_getdate("cli_message",SERVER_TIME_ID_GETDATE,&vDate);
//    printf("\ncli_message : result = %d date = %f \n",result,vDate);

    test_msgQueue();

    return EXIT_SUCCESS;
}
