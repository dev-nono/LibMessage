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
#include <poll.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <limits.h>


#include <fcntl.h>           /* Pour les constantes O_* */
#include <sys/stat.h>        /* Pour les constantes des modes */
#include <mqueue.h>

#include "apisyslog.h"
#include "utils.h"

#include "libmsg_srvtime.h"




int test_msgQueue(char *a_ID)
{
//#define SERVER_TIME "toto"

    int result = 0;
    int vLenReceive = 0;
    char vClientfilename[PATH_MAX] = "";
    char vServerfilename[PATH_MAX] = SERVER_TIME;
    struct mq_attr  vAttr   = {0};

    struct pollfd fd_client = {0};
    struct pollfd fd_server = {0};

    char    vBufferIN[HARD_MAX] = {0};
    char    vBufferOUT[HARD_MAX] = {0};

    (void)vServerfilename;
    (void)vBufferIN;

    vAttr.mq_flags  = O_CLOEXEC;
    vAttr.mq_curmsgs = 1;
    vAttr.mq_maxmsg = 1;
    vAttr.mq_msgsize = HARD_MAX;

    //**********************************************************
    // open mq server
    //**********************************************************
    fprintf(stderr,"_1_ \n");

    errno = 0;
    fd_server.fd = mq_open(SERVER_TIME,
            O_WRONLY); //,S_IRWXO | S_IRWXG | S_IRWXU ,&vAttr);

    if( -1 == fd_server.fd )
    {
        fprintf(stderr,"mq_open(SERVER_TIME) result=%d errno=%d %s \n",
                result , errno,strerror(errno));
    }
    fprintf(stderr,"_2_ \n");

    if( 0 == result )
    {
        //**********************************************************
        // open mq client
        //**********************************************************
        getMqClientname(a_ID,"getdate",vClientfilename);

        fprintf(stderr,"_3_ name=%s \n",vClientfilename);

        //mq_unlink(vClientfilename);

        errno = 0;
        fd_client.fd = mq_open(vClientfilename,
                O_CREAT | O_RDONLY , S_IRWXO | S_IRWXG | S_IRWXU ,&vAttr);
        if( -1 == fd_client.fd )
        {
            fprintf(stderr,"mq_open(%s) result=%d errno=%d %s \n",
                    vClientfilename,result , errno,strerror(errno));
        }
        fprintf(stderr,"_4_ \n");

        fushMessageQueue( fd_client.fd);
        // prepare msg

        do{
//            fprintf(stderr,"_5_ \n");
            // send msg to server
            //int mq_send(mqd_t mqdes, const char *msg_ptr,size_t msg_len, unsigned int msg_prio);

            result = mq_send(fd_server.fd, vClientfilename,
                    strlen(vClientfilename)+1,0);

//            result = mq_send(fd_server.fd, vClientfilename,
//                    strlen(vClientfilename)+1,0);

//            printf("type any key to continue. \n");
//            getchar();
            //            fprintf(stderr,"_6_ \n");

            if( 0 != result)
            {
                fprintf(stderr,"mq_send(%d) result=%d errno=%d %s \n",
                        fd_server.fd,result , errno,strerror(errno));
            }

            memset(vBufferOUT,0,sizeof(vBufferOUT));

            //receive msg in client
            vLenReceive =  mq_receive(fd_client.fd,
                    vBufferOUT,
                    HARD_MAX,
                    0U);

            double *pDbl = (double*)&vBufferOUT;
            printf("vLenReceive=%d data= %f\n",
                    vLenReceive,
                    *pDbl);

            sleep(1);
        }while(1);

    }

    return result;
}
int check_loop(const char* a_UniqID, const char* a_value)
{
    int     result = 0;
    double  vDate = 0.0;

    (void)a_value;
    do{
        vDate = 0.0;

        result =  libmsg_srvtime_cli_getdate(a_UniqID, &vDate);

        if( 0 == result )
        {
            fprintf(stdout,"date= %.9f result=%d \n",vDate,result);
        }
        else
        {
            fprintf(stdout,"date = error result=%d \n",result);
        }

        getchar();

    }while(1);

    return 0;
}

static int libmsg_cli_cbfcnt_signaldate(
        const sRequest_t  *a_pRequest,
        sResponse_t *a_pResponse)
{

    sSignaldateResponse_t *pSignaldateResponse = (sSignaldateResponse_t *)a_pResponse->data;

    TRACE_DBG4(" : date=%ld.%09ld \n",
            pSignaldateResponse->timespesc.tv_sec,
            pSignaldateResponse->timespesc.tv_nsec);


    return 0;
}

int check_signal(const char* a_UniqID, const char* a_value)
{
    int         result  = 0;
    double      timeout = 0.0;
    char        buffer[255] = {0};

    timeout = atof(a_value);

    result = libmsg_srvtime_cli_signaldate(a_UniqID,
            timeout,
            &libmsg_cli_cbfcnt_signaldate);

//    if( 0 == result )
//    {
//        result = libmsg_srvtime_cli_wait();
//    }
    do{

        memset(buffer,0,255);
        fgets(buffer,255,stdin);

       // if( buffer[])

    }while(1);

    return result;
}

int main(int argc, char* argv[])
{
    int     result = EXIT_SUCCESS;

    apisyslog_init("");

    TRACE_DBG1("main_")

//    result = libmessage_getdate("cli_message",SERVER_TIME_ID_GETDATE,&vDate);
//    printf("\ncli_message : result = %d date = %f \n",result,vDate);

    if(argc > 3 )
    {
        if( 0 == strcmp(argv[1] ,"loop") )
        {
            result =  check_loop(argv[2],"");
        }
        if( 0 == strcmp(argv[1] ,"signal") )
        {
            result =  check_signal(argv[2],argv[3]);
        }
    }
    else
    {
        fprintf(stderr,"\n syntaxe error : %s [cmd] [ID] [value] \n",argv[0]);
        fprintf(stderr," [cmd] : loop | signal \n");
        fprintf(stderr," [ID] : 1, 2 ,3. ... \n\n");
    }

    return result;
}// int main(int argc, char* argv[])
