/*
 ============================================================================
 Name        : cli_message.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>


#include "apisyslog.h"
#include "libmessage_svc_time.h"


#define MAX_ARRAY (10U)

int main(int argc, char *argv[])
{
    int     result = EXIT_SUCCESS;
    double  arrayDate[MAX_ARRAY] = {0.0};
    double  value = 0.0;
    int counter = -1;
    unsigned int ii = 0;


    apisyslog_init("");


    TRACE_IN("cli_message main IN")


    if( argc > 1 )
    {
        do{
            if( counter == MAX_ARRAY-1 )
            {
                counter = 0;

                for(ii=0; ii < MAX_ARRAY;ii++)
                {
                    printf("%0.9f\n",arrayDate[ii]);
                }

                memset(&arrayDate,0,sizeof(arrayDate));

                printf("\n%s type any key to continue \n",getStrDate());
                getchar();
            }
            else
            {
                counter++;
            }

            value = 0.0;
            result = libmessage_getdate(argv[1],&value);
            arrayDate[counter] = value;

        }while(1);
    }
    else
    {
        printf("Error no client name ! \n");
    }

    printf("exit result=%d ii=%d \n",result, ii);

    return result;
}
