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
#include <limits.h>
#include <string.h>
#include <errno.h>
 #include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "libmessage.h"



int check_fifo(const char* a_argv1)
{
    int result = 0;
    char    vClientName[NAME_MAX] = {0};
    int     fdServer = -1;
    int     fdClient = -1;
    char    buffer[1024] = {0};


    snprintf(vClientName,NAME_MAX-1,"/tmp/client.%s",a_argv1);

    //create client response RO
    errno = 0;
    result = mkfifo(vClientName,S_IRWXU);

//    res   errno
//    0     0       ok
//    0     1       ok
//    -1    EEXIST  ok
//    -1    xxx     error


    if( (0 != result ) && (EEXIST != errno) )
    {
        // error
        printf("Error %d: mkfifo(-%s-) %s \n",
                errno,vClientName,strerror(errno));
    }
    else
    {
        printf("client name:  -%s- \n",vClientName);
        result = 0;
    }

    if( 0 == result )
    {
        // open server endpoint  argv[1] WO
        errno = 0;
        fdServer = open(SVR_TIME_GETDATE,O_CLOEXEC);

        if( -1 == fdServer )
        {
            printf("Error %d: open(-%s-) %s \n",
                    errno,SVR_TIME_GETDATE,strerror(errno));
            result = errno;
        }
    }


    if( 0 == result )
    {
        do
        {
            printf("type any key to continue \n");
            getchar();

            //send request to server endpoint
            errno = 0;
            result = write(fdServer,"0",1);
            if(-1 ==  result)
            {
                printf("Error %d: write(-%s-) %s \n",
                        errno,SVR_TIME_GETDATE,strerror(errno));
            }
            else
            {
                errno = 0;
                result = read(fdClient,buffer,1024);

                if( 0 == result )
                {
                    printf("Error %d: read(-%s-) size == 0  \n",
                            errno,vClientName);
                }
                else if (-1 == result )
                {
                    printf("Error %d: read(-%s-) %s \n",
                            errno,vClientName,strerror(errno));
                }
                else
                {
                    printf("server response = %s  size=%d \n",buffer,result);
                }
            }

        }while(1);

        //close  server endpoint
       close(fdServer);
    }

    // read response from server

    return result;
}


int main(int argc, char *argv[])
{
    int     result = EXIT_SUCCESS;
    //double  vDate = 0.0;

    //result = libmessage_getdate("cli_message",SERVER_TIME_ID_GETDATE,&vDate);
//    printf("\ncli_message : result = %d date = %f \n",result,vDate);


    if( argc > 1 )
    {
        result = check_fifo(argv[1]);
    }
    else
    {
        printf("Error no client name ! \n");
    }
    return result;
}
