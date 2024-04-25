#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "hardware.h"

#define BLINK_GPIO 48u
#define BLINK_PERIOD 1000

static uint8_t s_led_state = 0;
static led_strip_handle_t led_strip;
static const char *TAG = "PH Main";


static void configure_led(void);
static void blink_led(void);
void hw_task(void *pvParameter);


void app_main(void)
{
    /* Initialize hardware */
	hardware_init();

	/* Create thread for hardware task */
	xTaskCreate(hw_task,
                "hardware_low_level_task",
                2048,
                NULL,
                1,
                NULL);

	/* Configure the peripheral according to the LED type */
    configure_led();

    uint16_t pot_value;
    uint8_t red;
    uint8_t green;
    uint8_t blue;

    while(1)
    {
    	vTaskDelay(50 / portTICK_PERIOD_MS);
    	pot_value = hardware_get_pot_value();

    	//pot_value *= 16;

    	//red = pot_value >> 11;
    	//green = (pot_value >> 5) & 0x3fu;
    	//blue = pot_value & 0x1Fu;

    	blue = 0;
    	red = 16 - (pot_value / 256);
    	green = pot_value / 256;

    	led_strip_set_pixel(led_strip, 0, red, green, blue);
    	/* Refresh the strip to send data */
        led_strip_refresh(led_strip);
    }

#if 0
    while (1)
    {
        blink_led();
        /* Toggle the LED state */
        s_led_state++;
        if (s_led_state > 5u)
        {
        	s_led_state = 0u;
        }
        vTaskDelay(BLINK_PERIOD / portTICK_PERIOD_MS);
    }
#endif
}


void hw_task(void *pvParameter)
{
  while(1)
  {
	  vTaskDelay(50 / portTICK_PERIOD_MS);
	  /* Call the thread every 20 milliseconds, hopefully... */
	  hardware_main();
  }
}


static void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink addressable LED!");
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = BLINK_GPIO,
        .max_leds = 1, // at least one LED on board
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);
}


static void blink_led(void)
{
    /* If the addressable LED is enabled */
	switch(s_led_state)
    {
		case 0:
		/* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
	    	led_strip_set_pixel(led_strip, 0, 0, 0, 16);
	    	break;
		case 1:
		/* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
	    	led_strip_set_pixel(led_strip, 0, 0, 16, 0);
	    	break;
		case 2:
		/* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
	    	led_strip_set_pixel(led_strip, 0, 16, 0, 0);
	    	break;
		case 3:
		/* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
	    	led_strip_set_pixel(led_strip, 0, 0, 8, 8);
	    	break;
		case 4:
		/* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
	    	led_strip_set_pixel(led_strip, 0, 16, 16, 0);
	    	break;
		case 5:
		/* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
	    	led_strip_set_pixel(led_strip, 0, 16, 0, 16);
	    	break;
    }

    /* Refresh the strip to send data */
            led_strip_refresh(led_strip);
}
