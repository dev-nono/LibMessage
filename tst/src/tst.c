/*
 ============================================================================
 Name        : tst.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <fcntl.h>           /* Pour les constantes O_* */
#include <sys/stat.h>        /* Pour les constantes des modes */
#include <mqueue.h>
#include <string.h>
#include <errno.h>


#include <stdio.h>
#include <stdlib.h>

#define MQ_FILENAME "/server_time"


//#define STRINGIFY(x) #x
//#define STRINGIFY_2(aa,bb) STRINGIFY(aa ## bb)
//#define STRINGIFY_3(aa,bb,cc) STRINGIFY(aa ## bb ## cc)
//
////#define CONCAT2( _a , _b )     _a _b
////#define CONCAT_SEM( _a , _b )  STRINGIFY( _a._b )
//
////#define CONCAT(  _a , _b )     CONCAT3( _a ,_b )
//
//
////#define  SVR_TIME() STRINGIFY(srvtime)
//#define  SVR_TIME           srvtime
//
//#define SVC_GETDATE         getdate
//#define SVC_SETDATE         setdate
//#define SVC_SIGNALDATE      signaldate
//
//#define SVR_TIME_GETDATE_CONCAT(aa,bb)   STRINGIFY_3( aa , . , bb )
//
//#define SVR_TIME_GETDATE                    SVR_TIME_GETDATE_CONCAT( SVR_TIME , SVC_GETDATE )
////#define SVR_TIME_SETDATE                    SVR_TIME_GETDATE_CONCAT( SVR_TIME , SVC_SETDATE )
////
////#define SVR_TIME_GETDATE_CONCAT_SEM(aa , bb)    STRINGIFY3( aa , _ , bb )
////#define SVR_TIME_GETDATE_SEM                   SVR_TIME_GETDATE_CONCAT_SEM( SVR_TIME,SVC_GETDATE )


#define STRINGIFY(x) #x
#define STRINGIFY_2(aa,bb)      STRINGIFY(aa ## bb)
#define STRINGIFY_3(aa,bb,cc)   STRINGIFY(aa ## bb ## cc)

#define CONCAT2(_a,_b) _a _b
#define CONCAT_SEM(_a,_b) STRINGIFY( _a._b )





#define SVR_TIME srvtime

#define SVC_GETDATE getdate
#define SVC_SETDATE setdate
#define SVC_SIGNALDATE signaldate

#define CONCAT_POINT(_a,_b) STRINGIFY( _a._b ) // ok "."
#define CONCAT_U(aa,bb) STRINGIFY_3( aa , _ , bb ) // ok "_"


#define SVR_TIME_GETDATE CONCAT_POINT( SVR_TIME,SVC_GETDATE )
#define SVR_TIME_SETDATE CONCAT_POINT( SVR_TIME,SVC_SETDATE )

#define SVR_TIME_GETDATE_SEM CONCAT_U( SVR_TIME,SVC_GETDATE )





int check_preprocessor()
{

    //printf("%s  \n",SVR_TIME_GETDATE_SEM);
    printf("%s  \n",SVR_TIME_GETDATE);
    printf("%s  \n",SVR_TIME_SETDATE);
    printf("%s  \n",SVR_TIME_GETDATE_SEM);
    return 0;
}

int check_mq(void) {

    int result = 0;
    struct mq_attr  vAttr   = {0};

    vAttr.mq_flags  = O_CLOEXEC;
    vAttr.mq_curmsgs = 9;
    vAttr.mq_maxmsg = 9;
    vAttr.mq_msgsize = 1024;

    errno = 0;
    //result = mq_open(MQ_FILENAME, O_CREAT,S_IRWXG,&vAttr);
//    result = mq_open(MQ_FILENAME, O_CREAT,S_IRWXG,0); // ok
    result = mq_open(MQ_FILENAME, O_CREAT,S_IRWXO | S_IRWXG | S_IRWXU ,&vAttr); // ok

    printf("mq_open(%s) result=%d errno=%d %s\n",
            MQ_FILENAME,result,errno, strerror(errno));


    return EXIT_SUCCESS;
}

int main(void)
{

    // check_mq();
    check_preprocessor();

}
