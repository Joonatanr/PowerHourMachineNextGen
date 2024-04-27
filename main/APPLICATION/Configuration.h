/*
 * Configuration.h
 *
 *  Created on: 27 Dec 2023
 *      Author: Joonatan
 */

#ifndef LOGIC_CONFIGURATION_H_
#define LOGIC_CONFIGURATION_H_

#include "typedefs.h"

typedef enum
{
    CONFIG_ITEM_TASK_FREQ,
    CONFIG_ITEM_COLOR_SCHEME,
    CONFIG_ITEM_BRIGHTNESS,
    CONFIG_ITEM_BUZZER,

    NUMBER_OF_CONFIG_ITEMS
} Configuration_Item;

extern void configuration_start(void);

extern U32 configuration_getItem(Configuration_Item item);
extern void configuration_setItem(U32 value, Configuration_Item item);

#endif /* LOGIC_CONFIGURATION_H_ */
