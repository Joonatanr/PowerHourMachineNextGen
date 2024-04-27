/*
 * buttons.c
 *
 *  Created on: Sep 7, 2017
 *      Author: Joonatan
 */


/*****************************************************************************************************
 *
 * Imported definitions
 *
 *****************************************************************************************************/

#include "buttons.h"
#include "driver/gpio.h"
#include "xtensa/xtruntime.h"

/*****************************************************************************************************
 *
 * Private constant definitions
 *
 *****************************************************************************************************/
#define BUTTON_HOLD_TIME 3000u  /* Set this threshold to 3 seconds. */

#define ENTER_CRITICAL_SECTION() 	uint32_t volatile register ilevel = XTOS_DISABLE_ALL_INTERRUPTS
#define EXIT_CRITICAL_SECTION()		XTOS_RESTORE_INTLEVEL(ilevel)



/*****************************************************************************************************
 *
 * Private type definitions
 *
 *****************************************************************************************************/
typedef Boolean (*buttonFunction)(void);


typedef struct
{
    uint32_t btn_pin;

    ButtonMode     mode;                  //Rising or falling edge.
    buttonListener listener_press_func;   //Can be subscribed to. > Triggers when button is pressed
    buttonListener listener_hold_func;    //Can be subscribed to. > Triggers when button is held down for a period of time.

    Boolean        button_hold; /* Used for falling edge detection.     */
    U16            hold_cnt;    /* Used for button held-down detection  */

    Boolean        pressed;     /* Flag is triggered when a button press has been detected.         */
    Boolean        held_down;   /* Flag is triggered when a button has been held down for a time    */
} ButtonState;


/*****************************************************************************************************
 *
 * Private variable declarations
 *
 *****************************************************************************************************/

Private const uint32_t priv_button_config[NUMBER_OF_BUTTONS] =
{
		39u,      /* BLUE_BUTTON       */
		38u,      /* YELLOW_BUTTON     */
		47u,      /* RED_BUTTON        */
		41u,      /* GREEN_BUTTON      */
};

Private ButtonState priv_button_state[NUMBER_OF_BUTTONS];


/*****************************************************************************************************
 *
 * Public function definitions
 *
 *****************************************************************************************************/
//Set up buttons as inputs.
Public void buttons_init(void)
{
    U8 ix;

    for (ix = 0u; ix < NUMBER_OF_BUTTONS; ix++)
    {
        priv_button_state[ix].btn_pin = priv_button_config[ix];

        priv_button_state[ix].listener_hold_func = NULL;
        priv_button_state[ix].listener_press_func = NULL;

        priv_button_state[ix].held_down =   FALSE;
        priv_button_state[ix].pressed =     FALSE;
        priv_button_state[ix].button_hold = FALSE;
        priv_button_state[ix].mode = FALLING_EDGE;

        gpio_set_direction(priv_button_config[ix], GPIO_MODE_INPUT);
    }
}


//Hi priority task.
Public void buttons_cyclic10msec(void)
{
    U8 ix;
    Boolean state;
    ButtonState * btn_ptr;

    for (ix = 0u; ix < NUMBER_OF_BUTTONS; ix++)
    {
        btn_ptr = &priv_button_state[ix];
        state = (gpio_get_level(btn_ptr->btn_pin) == 1u) ? FALSE : TRUE;

        /* Handle button press detection. */
        if (state && (btn_ptr->button_hold == FALSE))
        {
            btn_ptr->button_hold = TRUE;
            if (btn_ptr->mode == RISING_EDGE)
            {
                btn_ptr->pressed = TRUE;
            }
        }
        else if (!state && (btn_ptr->button_hold == TRUE))
        {
            btn_ptr->button_hold = FALSE;
            if (btn_ptr->mode == FALLING_EDGE)
            {
                btn_ptr->pressed = TRUE;
            }
        }
        else
        {
            /* Do nothing. */
        }

        /* Handle button hold-down detection */
        if (state)
        {
            btn_ptr->hold_cnt += 10u;
        }
        else
        {
            btn_ptr->hold_cnt = 0u;
        }

        if (btn_ptr->hold_cnt >= BUTTON_HOLD_TIME)
        {
            btn_ptr->held_down = TRUE;
        }
    }
}


//Low priority task.
Public void buttons_cyclic100msec(void)
{
    U8 ix;
    for (ix = 0u; ix < NUMBER_OF_BUTTONS; ix++)
    {
        if (priv_button_state[ix].pressed)
        {
            priv_button_state[ix].pressed = FALSE;
            if (priv_button_state[ix].listener_press_func != NULL)
            {
                priv_button_state[ix].listener_press_func();
            }
        }

        if (priv_button_state[ix].held_down)
        {
            priv_button_state[ix].held_down = FALSE;
            if (priv_button_state[ix].listener_hold_func != NULL)
            {
                priv_button_state[ix].listener_hold_func();
            }
        }
    }
}


//Note that the listeners are called from lo prio context.
//This function subscribes a listener function to a button.
Public void buttons_subscribeListener(ButtonType btn, buttonListener listener)
{
    //Critical section.
	ENTER_CRITICAL_SECTION();
	if (btn < NUMBER_OF_BUTTONS)
	{
        priv_button_state[btn].listener_press_func = listener;
    }
	EXIT_CRITICAL_SECTION();
}


Public void buttons_setButtonMode(ButtonType btn, ButtonMode mode)
{
    if ((btn < NUMBER_OF_BUTTONS) && (mode < NUMBER_OF_BUTTON_MODES))
    {
        priv_button_state[btn].mode = mode;
    }
}


Public void buttons_subscribeHoldDownListener(ButtonType btn, buttonListener listener)
{
    ENTER_CRITICAL_SECTION();
    //Critical section.
    if (btn < NUMBER_OF_BUTTONS)
    {
        priv_button_state[btn].listener_hold_func = listener;
    }
    EXIT_CRITICAL_SECTION();
}


Public void buttons_unsubscribeAll(void)
{
    U8 ix;

    ENTER_CRITICAL_SECTION();
    for (ix = 0u; ix < NUMBER_OF_BUTTONS; ix++)
    {
        priv_button_state[ix].listener_press_func = NULL;
        priv_button_state[ix].listener_hold_func = NULL;
        priv_button_state[ix].mode = FALLING_EDGE; //This is default.
    }
    EXIT_CRITICAL_SECTION();
}


Public Boolean isButton(ButtonType btn)
{
    Boolean res = FALSE;

    if (btn < NUMBER_OF_BUTTONS)
    {
    	res = (gpio_get_level(priv_button_config[btn]) == 1u) ? FALSE : TRUE;
    }

    return res;
}


