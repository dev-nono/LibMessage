/*
 ============================================================================
 Name        : server_time.c
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
    int result = 0;

    result = libmessage_server_register(SERVER_TIME_GETDATE);
    result = libmessage_server_wait();

    return result;
}
