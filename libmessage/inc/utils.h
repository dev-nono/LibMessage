/*
 * utils.h
 *
 *  Created on: 10 juil. 2020
 *      Author: bruno
 */

#ifndef COMMON_INC_UTILS_H_
#define COMMON_INC_UTILS_H_

#include <time.h>

//*******************************************************************
//*
//*     CONSTANTES
//*
//*******************************************************************
#define DOUBLE_MANTIS_SIZE   (30U)
#define NS_PER_SECOND (1000000000LL)


#define FAILURE    1   /* Failing  status.  */
#define SUCCESS    0   /* Successful status.  */

//*******************************************************************
//*
//*             MACRO
//*
//*******************************************************************
#define STRINGIFY(x) #x
#define STRINGIFY_2(aa,bb)      STRINGIFY(aa ## bb)
#define STRINGIFY_3(aa,bb,cc)   STRINGIFY(aa ## bb ## cc)

#define CONCAT_POINT(_a,_b) STRINGIFY( _a._b )          // "." for semaphore filename
#define CONCAT_SEM(aa,bb) STRINGIFY_3( aa , _ , bb )    // "_" for fifo filename


//void add_timespec(struct timespec t1, struct timespec t2, struct timespec *td)
//*****************************************************
// ADD_TIMESPEC(t1,t2,td)
//      t1  : current date
//      t2  : delta to add
//      td  : new date calculated
//*****************************************************
#define ADD_TIMESPEC(t1,t2,td) \
do{ \
    td.tv_nsec = t2.tv_nsec + t1.tv_nsec;\
    td.tv_sec  = t2.tv_sec + t1.tv_sec; \
    if (td.tv_nsec >= NS_PER_SECOND) \
    {\
        td.tv_nsec -= NS_PER_SECOND; \
        td.tv_sec++; \
    }\
    else if (td.tv_nsec <= -NS_PER_SECOND)\
    {\
        td.tv_nsec += NS_PER_SECOND;\
        td.tv_sec--;\
    }\
}while(0);


//*****************************************************
// ADD_TIMESPEC_REAL(t1,td)
//      t1  : delta to add
//      td  : new date calculated
//*****************************************************
#define ADD_TIMESPEC_REAL(t1,td) \
do{ \
    struct timespec vTmpDate = {0,0}; \
\
    clock_gettime(CLOCK_REALTIME, &vTmpDate); \
 \
    td.tv_nsec = vTmpDate.tv_nsec + t1.tv_nsec; \
    td.tv_sec  = vTmpDate.tv_sec + t1.tv_sec; \
    if (td.tv_nsec >= NS_PER_SECOND) \
    {\
        td.tv_nsec -= NS_PER_SECOND; \
        td.tv_sec++; \
    }\
    else if (td.tv_nsec <= -NS_PER_SECOND)\
    {\
        td.tv_nsec += NS_PER_SECOND;\
        td.tv_sec--;\
    }\
}while(0);

//*****************************************************
// ADD_TIMESPEC_RAW(t1,td)
//      t1  : delta to add
//      td  : new date calculated
//*****************************************************
#define ADD_TIMESPEC_RAW(t1,td) \
do{ \
    struct timespec vdate = {0,0};\
\
    clock_gettime(CLOCK_MONOTONIC_RAW, &vdate);\
 \
    td.tv_nsec = vdate.tv_nsec + t1.tv_nsec;\
    td.tv_sec  = vdate.tv_sec + t1.tv_sec; \
    if (td.tv_nsec >= NS_PER_SECOND) \
    {\
        td.tv_nsec -= NS_PER_SECOND; \
        td.tv_sec++; \
    }\
    else if (td.tv_nsec <= -NS_PER_SECOND)\
    {\
        td.tv_nsec += NS_PER_SECOND;\
        td.tv_sec--;\
    }\
}while(0);


//*******************************************************************
//*
//*
//*
//*******************************************************************
const char *    getDateRawStr();
const char *    getDateRaw(char *a_BufferDate);
double          getDateRawDouble();

int getFifoname(char* a_Fifoname);
int getProcessname(char* a_Processname);

#endif /* COMMON_INC_UTILS_H_ */

