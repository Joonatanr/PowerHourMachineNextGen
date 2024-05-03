/*
 * systimer.c
 *
 *  Created on: 14 Dec 2023
 *      Author: Joonatan
 */

#include "systimer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

volatile U32 priv_systimer2_cnt = 0;

/* Major TODO : Systimer is currently broken and does not do what is expected. However
 * the system itself still works. */

Public void systimer_init(void)
{
	/* Do nothing here... */
}


Public U32 systimer_getTimestamp(void)
{
	return xTaskGetTickCount() / portTICK_PERIOD_MS;
}


Public U32 systimer_getPeriod(U32 start_time)
{
    return systimer_getTimestamp() - start_time;
}


Public void systimer2_increment(void)
{
	priv_systimer2_cnt++;
}

Public U32 systimer2_getTimer(void)
{
	return priv_systimer2_cnt;
}
