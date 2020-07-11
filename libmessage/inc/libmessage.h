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

typedef int (*libmessage_pFunctCB_t)(char*);

//******************************************************
//  generic function
//******************************************************
const char * getDateRawStr();


#endif /* INC_LIBMESSAGE_H_ */
