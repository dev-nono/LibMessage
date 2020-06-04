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

// libmessage.h
#include <limits.h> // libmessage.h

#include "libmessage.h"


int main(void)
{
    int     result = 0;
    double  vDate = 0.0;

    result = server_time_getdate("cli_message",&vDate);

    printf("\ncli_message : result = %d date = %f \n",result,vDate);



    return EXIT_SUCCESS;
}
