/*
 * pot.h
 *
 *  Created on: 16. dets 2019
 *      Author: JRE
 */

#ifndef HW_POT_H_
#define HW_POT_H_

#include "typedefs.h"


typedef enum
{
    POTENTIOMETER_NUDE_LEVEL,       /* Stripi m��dik                        */
    POTENTIOMETER_SEXY_LEVEL,       /* Nilbuste m��dik                      */
    POTENTIOMETER_GIRLS,            /* How brave/intense the girls are      */
    POTENTIOMETER_GUYS,             /* How brave/intense the guys are       */

    NUMBER_OF_DEFINED_POTENTIOMETERS
} potentiometer_T;

#define POTENTIOMETER_1 POTENTIOMETER_GIRLS
#define POTENTIOMETER_2 POTENTIOMETER_GUYS
#define POTENTIOMETER_3 POTENTIOMETER_SEXY_LEVEL
#define POTENTIOMETER_4 POTENTIOMETER_NUDE_LEVEL

typedef void (*pot_value_changed_func)(void);

extern void pot_init(void);
extern void pot_cyclic_100ms(void);
extern int  pot_getSelectedRange(potentiometer_T pot);
extern int pot_getCurrentMeasurement(potentiometer_T pot);

extern void pot_register_callback(pot_value_changed_func func);
extern void pot_unregister_callback(void);

#endif /* HW_POT_H_ */
