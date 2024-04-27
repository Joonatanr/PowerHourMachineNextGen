/*
 * systimer.c
 *
 *  Created on: 14 Dec 2023
 *      Author: Joonatan
 */

#include "systimer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


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
