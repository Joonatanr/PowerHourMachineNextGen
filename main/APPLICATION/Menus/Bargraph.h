/*
 * Scrollbar.h
 *
 *  Created on: Mar 21, 2018
 *      Author: Joonatan
 */

#ifndef LOGIC_MENUS_BARGRAPH_H_
#define LOGIC_MENUS_BARGRAPH_H_

#include "typedefs.h"
#include "Configuration.h"
#include "menu.h"

typedef void (*bargraph_func)(U16 value);


typedef struct __Bargraph__
{
    U16 value;
    U16 max_value;
    U16 min_value;
    U16 increment;
    struct _Selection_Menu_ * parent;
    char * text;
    bargraph_func value_changed;
    Configuration_Item config_item;

    U16 x_loc;
    U16 y_loc;
    U16 width;
} Bargraph_T;

extern Bargraph_T BRIGHTNESS_BARGRAPH;
extern Bargraph_T SNAKE_SPEED_BARGRAPH;
extern Bargraph_T TASK_FREQUENCY_BARGRAPH;

extern void enterBarGraph(Bargraph_T * bar);


/* Placeholder functions, these should be moved elsewhere once we have implemented something. */
extern void clockDisplay_setTaskFrequency(U16 value);
extern U16 clockDisplay_getTaskFrequency(void);
extern void snake_setSpeed(U16 value);

#endif /* LOGIC_MENUS_BARGRAPH_H_ */
