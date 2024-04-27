/*
 * hardware.c
 *
 *  Created on: Apr 25, 2024
 *      Author: JRE
 */

/*****************************************************************************************************
 *
 * Imported definitions
 *
 *****************************************************************************************************/
#include "hardware.h"
#include "buttons.h"
#include "pot.h"
#include "backlight.h"

#include "esp_log.h"
#include "esp_timer.h"

/*****************************************************************************************************
 *
 * Private constant definitions
 *
 *****************************************************************************************************/


/*****************************************************************************************************
 *
 * Private variable declarations
 *
 *****************************************************************************************************/


Private const char *TAG = "PH Hardware";




/*****************************************************************************************************
 *
 * Private function forward declarations
 *
 *****************************************************************************************************/


Private void configure_timer(void);
void timer_callback_10msec(void *param);


/*****************************************************************************************************
 *
 * Public function definitions
 *
 *****************************************************************************************************/
Public void hardware_init(void)
{
	configure_timer();

	buttons_init();

	pot_init();

	backlight_init();
}

/* Runs every 100 ms. */
Public void hardware_main(void)
{
	/* Run the ADC measurements for the potentiometers. */
	pot_cyclic_100ms();
}

/*****************************************************************************************************
 *
 * Private function definitions
 *
 *****************************************************************************************************/
Private void configure_timer(void)
{
	const esp_timer_create_args_t my_timer_args =
	{
	      .callback = &timer_callback_10msec,
	      .name = "My Timer"
	};

	esp_timer_handle_t timer_handler;

	printf("Configuring timer...\n");

	ESP_ERROR_CHECK(esp_timer_create(&my_timer_args, &timer_handler));
	ESP_ERROR_CHECK(esp_timer_start_periodic(timer_handler, 10000)); /* Set timer to 10000 microseconds, (10ms) */
}


/* TODO : Here we are going to run the realtime hi prio thread. */
void timer_callback_10msec(void *param)
{
	buttons_cyclic10msec();
}


