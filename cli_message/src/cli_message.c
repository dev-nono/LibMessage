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
#include <libmsg_srvtimer.h>
#include <sys/stat.h>        /* Pour les constantes des modes */

#include "apisyslog.h"
#include "utilstools_time.h"


static int cli_cbfcnt_signaldate(sResponse_t *a_pResponse)
{

    sTimerResponse_t *pSignaldateResponse = (sTimerResponse_t *)a_pResponse->data;

    printf("%s : date=%ld.%09ld \n",
            __FUNCTION__,
            pSignaldateResponse->timespesc.tv_sec,
            pSignaldateResponse->timespesc.tv_nsec);


    return 0;
}

int check_loop( const char* a_value)
{
    int     result = 0;
    double  vDate = 0.0;
    double     dVal = 0;
    int         val = 0;

    dVal = atof(a_value);
    val = dVal * 1e6;
    //    printf("a_value=%s dval=%f val=%d \n",a_value,dVal,val);

    do{
        vDate = 0.0;

        result =  libmsg_srvtimer_cli_getdate(SRVTIMER_GETDATE,&vDate);

        if( 0 == result )
        {
            fprintf(stdout,"date= %.9f result=%d \n",vDate,result);
        }
        else
        {
            fprintf(stdout,"date = error result=%d \n",result);
        }

        if( 0 == dVal)
        {
            getchar();
        }
        else
        {
            usleep( val);
        }

    }while(1);

    return 0;
}



int check_signal(const char* a_value)
{
    int         result  = 0;
    double      timeout = 0.0;
    struct timespec timeout_ts = {0};

    sSignal_t dataSvcTimer = {0};

    timeout = atof(a_value);

    time_cnv_double_to_ts(timeout,&timeout_ts);

    result = libmsg_srvtimer_cli_timer(SRVTIMER_TIMER,timeout_ts,cli_cbfcnt_signaldate,&dataSvcTimer);

    printf("%s_ : result = %d \n",__FUNCTION__,result);

    getchar();

    close(dataSvcTimer.socket);
    unlink(dataSvcTimer.filenameClient);


    return result;
}

void printUsage(char *a_argv0 )
{
    fprintf(stderr,"\n syntaxe error : %s [cmd] [value] \n",a_argv0);
    fprintf(stderr," [cmd] : loop | signal \n\n");

}
int main(int argc, char* argv[])
{
    int     result = EXIT_SUCCESS;

    apisyslog_init("");

    TRACE_DBG1("main_")

    if(argc > 2)
    {
        if( 0 == strcmp(argv[1] ,"signal") )
        {
            result =  check_signal(argv[2]);
        }

        else if( 0 == strcmp(argv[1] ,"loop") )
        {
            result =  check_loop(argv[2]);
        }
        else
        {
            printUsage(argv[0]);
        }
    }
    else
    {
        printUsage(argv[0]);
    }

    return result;
}// int main(int argc, char* argv[])
