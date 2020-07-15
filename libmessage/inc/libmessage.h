/*
 * libmessage.h
 *
 *  Created on: 4 juin 2020
 *      Author: bruno
 */

#ifndef INC_LIBMESSAGE_H_
#define INC_LIBMESSAGE_H_


#define _IN_
#define _OUT_
#define _INOUT_

enum eLIBMSG_ID
{
    eLIBMSG_ID_GETDATA = 0,
    eLIBMSG_ID_SETDATA,
    eLIBMSG_ID_SIGNAL,
    eLIBMSG_ID_END
};

typedef enum eLIBMSG_ID eLIBMSG_ID_t;


typedef int (*libmessage_pFunctCB_t)(const void * , void *);

//******************************************************
//  generic function
//******************************************************

#endif /* INC_LIBMESSAGE_H_ */
