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

//  include ==> libmessage.h
#include <limits.h> // libmessage.h

// include ==> #include libmessage_int.h
#include <mqueue.h>
// #include <limits.h>
#include "libmessage_int.h"

#include "libmessage.h"


//*********************************************************
//*
//*********************************************************
int main(void)
{
    int result = 0;

    result = libmessage_init();

    result = libmessage_server_time_register();
    result = libmessage_server_wait();

    return result;
}
