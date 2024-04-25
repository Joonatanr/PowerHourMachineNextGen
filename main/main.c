#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define BLINK_GPIO 48u
#define BLINK_PERIOD 1000

static uint8_t s_led_state = 0;
static led_strip_handle_t led_strip;
static const char *TAG = "example";


static void configure_led(void);
static void blink_led(void);


void app_main(void)
{
    /* Configure the peripheral according to the LED type */
    configure_led();

    while (1)
    {
        ESP_LOGI(TAG, "Turning the LED %s!", s_led_state == true ? "ON" : "OFF");
        blink_led();
        /* Toggle the LED state */
        s_led_state++;
        if (s_led_state > 5u)
        {
        	s_led_state = 0u;
        }
        vTaskDelay(BLINK_PERIOD / portTICK_PERIOD_MS);
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
