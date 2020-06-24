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

// include ==>  libmessage.h
#include "libmessage.h"


int main(void)
{
    int     result = 0;
    double  vDate = 0.0;

<<<<<<< HEAD
    result = libmessage_getdate("cli_message",SERVER_TIME_ID_GETDATE,&vDate);
=======
    result = libmessage_getdate("cli_message",SERVER_TIME_GETDATE,&vDate);
>>>>>>> branch 'master' of https://github.com/dev-nono/LibMessage.git

    printf("\ncli_message : result = %d date = %f \n",result,vDate);



    return EXIT_SUCCESS;
}
