 /*
 * utils.c
 *
 *  Created on: 10 juil. 2020
 *      Author: bruno
 */

#include <unistd.h>
#include <sys/syscall.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <libgen.h>

#include "utils.h"

//          1         2         3
// 1234567890123456789012345678901234567890
//


//****************************************************
//*
//*
//****************************************************
const char * getDateRawStr(char *a_BufferDate )
//const char * getDateRawStr()
{
    int             result = 0;
    char            vBuffer[DOUBLE_MANTIS_SIZE] = {0};
    struct timespec vDate= {0,0};

    if ( a_BufferDate)
    {
        result = clock_gettime(CLOCK_MONOTONIC_RAW,&vDate);
        result = DOUBLE_MANTIS_SIZE - 3;

        snprintf(vBuffer,result, "%4ld.%09ld",vDate.tv_sec,vDate.tv_nsec);

       strncpy(a_BufferDate,vBuffer,DOUBLE_MANTIS_SIZE-1);

       a_BufferDate[DOUBLE_MANTIS_SIZE-1] = 0;
    }

    return a_BufferDate;
}
//****************************************************
//*
//*
//****************************************************
double getDateRawDouble()
{
    double             result = 0;
    struct timespec vDate= {0,0};

    result = clock_gettime(CLOCK_MONOTONIC_RAW,&vDate);

    result = vDate.tv_sec;
    result += (vDate.tv_nsec*1.0) * (double)(1e-9 * 1.0);

    return result;
}

//****************************************************
//*
//*
//****************************************************
int getFifoname(char* a_Fifoname)
{
    int             result = 0;
    char vProcessname[NAME_MAX];

    result = getProcessname(vProcessname);

    int long vTID  = syscall(SYS_gettid);

    snprintf(a_Fifoname,PATH_MAX-1,"/tmp/%s.%ld",vProcessname,vTID);

    return result;
}

//****************************************************
//*
//*
//****************************************************
int getProcessname(char* a_Processname)
{
    int             result = 0;
    char vLinkname[PATH_MAX];
    char *pBasename = 0;


    result = readlink("/proc/self/exe", vLinkname, PATH_MAX-1);

    pBasename = basename(vLinkname);

    strncpy(a_Processname,pBasename,NAME_MAX-1);

    return result;
}


