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

    //*****************************
    // create new tread for listening incomming messages
    //*****************************

	libmessage_register_service();

	return EXIT_SUCCESS;
}
