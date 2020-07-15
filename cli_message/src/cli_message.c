/*
 ============================================================================
 Name        : cli_message.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */


#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>


#include "utils.h"
#include "apisyslog.h"

#include "libmessage_svc_time.h"


#define MAX_ARRAY (10U)


int check_getDateLoop(char* a_fifoName)
{
    int     result = SUCCESS;
    double  arrayDate[MAX_ARRAY] = {0.0};
    double  value = 0.0;
    int counter = -1;
    unsigned int ii = 0;


    TRACE_IN("cli_message main IN")


    do{
        if( counter == MAX_ARRAY-1 )
        {
            counter = 0;

            for(ii=0; ii < MAX_ARRAY;ii++)
            {
                printf("%0.9f\n",arrayDate[ii]);
            }

            memset(&arrayDate,0,sizeof(arrayDate));

            printf("\n%0.9f type any key to continue \n",getDateRawDouble());
            getchar();
        }
        else
        {
            counter++;
        }

        value = 0.0;
        result = libmessage_getdate(a_fifoName,&value);
        arrayDate[counter] = value;

    }while(1);

    printf("exit result=%d ii=%d \n",result, ii);

    return result;

}
int check_getDateSleep(char* a_fifoName)
{
    int     result = SUCCESS;
    double  value = 0.0;

    TRACE_IN("cli_message main IN")

    do{
        sleep(1);

        value = 0.0;
        result = libmessage_getdate(a_fifoName,&value);
        printf("%0.9f\n",value);

    }while(1);


    return result;
}

int check_setDateSleep(char* a_fifoName)
{
    int     result = SUCCESS;
    double  value = 0.0;

    TRACE_IN("cli_message main IN")

    do{
        sleep(1);

        value = 1.1234567890123;
        result = libmessage_setdate(a_fifoName,value);

        printf("%s : %d %s \n",__FUNCTION__,
                result,strerror(result));

    }while(1);


    return result;
}

int main(int argc, char *argv[])
{
    int     result = SUCCESS;

    apisyslog_init("");

    if( argc > 1 )
    {
        //check_getDateSleep(argv[1]);
        //check_getDateLoop(argv[1]);
        check_setDateSleep(argv[1]);
    }
    else
    {
        printf("Error no client name ! \n");
    }


    return result;
}
