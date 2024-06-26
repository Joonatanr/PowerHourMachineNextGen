/*
 * buzzer.c
 *
 *  Created on: Sep 7, 2017
 *      Author: Joonatan
 */


#include "buzzer.h"
#include "driver/gpio.h"

#define BUZZER_PIN 48
#define BEEP_INTERVAL 5u /* 400msec */
#define BEEP_LENGTH 3u   /* 200msec */

/* NOTE : Buzzer should be connected to the P2.3 port, because it can drive up to 20mA.
 * Not sure about other ports, these might be limited to 6mA */


Private int priv_buzzer_counter = -1;
Private int priv_beep_counter = 0;
Private U16 priv_cycle_counter;

Public void buzzer_init(void)
{
	gpio_set_direction(BUZZER_PIN, GPIO_MODE_OUTPUT);
	buzzer_setBuzzer(FALSE);
}


Public void buzzer_setBuzzer(Boolean state)
{
    if (state)
    {
        gpio_set_level(BUZZER_PIN, 1);
    }
    else
    {
    	gpio_set_level(BUZZER_PIN, 0);
    }
}


Public void buzzer_playBuzzer(U8 cycles)
{
    priv_buzzer_counter = cycles;
    buzzer_setBuzzer(TRUE);
}

Public void buzzer_playBeeps(U8 beeps)
{
    priv_beep_counter = beeps;
}


//Must be called every 100 msec.
Public void buzzer_cyclic100msec(void)
{
    priv_cycle_counter++;

    if (priv_beep_counter > 0)
    {
        if ((priv_cycle_counter % BEEP_INTERVAL) == 0)
        {
            buzzer_playBuzzer(BEEP_LENGTH);
            priv_beep_counter--;
        }
    }

    if (priv_buzzer_counter > 0)
    {
        priv_buzzer_counter--;
    }

    if(priv_buzzer_counter == 0)
    {
        priv_buzzer_counter = -1;
        buzzer_setBuzzer(FALSE);
    }
}


