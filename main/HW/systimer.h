/*
 * systimer.h
 *
 *  Created on: 14 Dec 2023
 *      Author: Joonatan
 */

#ifndef HW_SYSTIMER_H_
#define HW_SYSTIMER_H_

#include "typedefs.h"

extern void systimer_init(void);
extern U32 systimer_getTimestamp(void);
extern U32 systimer_getPeriod(U32 start_time);


#endif /* HW_SYSTIMER_H_ */
