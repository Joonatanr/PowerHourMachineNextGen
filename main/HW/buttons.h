/*
 * buttons.h
 *
 *  Created on: Sep 7, 2017
 *      Author: Joonatan
 */

#ifndef HW_BUTTONS_H_
#define HW_BUTTONS_H_

#include "typedefs.h"

typedef void (*buttonListener)(void);

typedef enum
{
    RISING_EDGE,
    FALLING_EDGE,
    NUMBER_OF_BUTTON_MODES
} ButtonMode;


/* Configuration part - Change this to suit the application. */
typedef enum
{
    BUTTON_RED,
    BUTTON_BLUE,
    BUTTON_BLACK,
    BUTTON_GREEN,

    NUMBER_OF_BUTTONS
} ButtonType;

#define UP_BUTTON       BUTTON_GREEN
#define LEFT_BUTTON     BUTTON_RED
#define RIGHT_BUTTON    BUTTON_BLUE
#define DOWN_BUTTON     BUTTON_BLACK

#define OK_BUTTON       LEFT_BUTTON
#define CANCEL_BUTTON   RIGHT_BUTTON

/**************************************************************/


extern void buttons_init(void);
extern void buttons_cyclic10msec(void);
extern void buttons_cyclic100msec(void);
extern void buttons_subscribeListener(ButtonType btn, buttonListener listener);
extern void buttons_setButtonMode(ButtonType btn, ButtonMode mode);
extern void buttons_subscribeHoldDownListener(ButtonType btn, buttonListener listener);
extern void buttons_unsubscribeAll(void);

extern Boolean isButton(ButtonType btn);


#endif /* HW_BUTTONS_H_ */
