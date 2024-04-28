/*
 * PowerHourMain.h
 *
 *  Created on: 13 Dec 2023
 *      Author: Joonatan
 */

#ifndef LOGIC_POWERHOURMAIN_H_
#define LOGIC_POWERHOURMAIN_H_


#include "typedefs.h"


extern void powerHour_init(void);
extern void powerHour_start(void);
extern void powerHour_cyclic1000msec(void);
extern void powerHour_cyclic100msec(void);
extern void powerHour_stop(void);

extern void powerHour_setTaskFrequency(U16 freq);
extern U16 powerHour_getTaskFrequency(void);
extern void setBuzzerEnable(U16 enable);
extern U16 isBuzzerEnabled(void);

#endif /* LOGIC_POWERHOURMAIN_H_ */
