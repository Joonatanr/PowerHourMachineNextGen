

#include "typedefs.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "hardware.h"
#include "buttons.h"
#include "pot.h"

#define BLINK_GPIO 48u
#define BLINK_PERIOD 1000

static led_strip_handle_t led_strip;
static const char *TAG = "PH Main";


static void configure_led(void);
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

    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;

#if 1
    while(1)
    {
    	vTaskDelay(50 / portTICK_PERIOD_MS);
    	vTaskDelay(1000 / portTICK_PERIOD_MS);

    	if (isButton(BUTTON_RED))
    	{
    		red = 16;
    	}
    	else if(isButton(BUTTON_BLUE))
    	{
    		blue = 16;
    	}
    	else if (isButton(BUTTON_GREEN))
    	{
    		green = 16;
    	}
    	else if (isButton(BUTTON_BLACK))
    	{
    		green = 16;
    		red = 16;
    		blue = 16;
    	}
    	else
    	{
    		/* Test the adc */
    		red = pot_getSelectedRange(POTENTIOMETER_1) * 4;
    		green = pot_getSelectedRange(POTENTIOMETER_2) * 4;
    		blue = pot_getSelectedRange(POTENTIOMETER_3) * 4;
    	}

    	led_strip_set_pixel(led_strip, 0, red, green, blue);
    	/* Refresh the strip to send data */
        led_strip_refresh(led_strip);
    }
#else

    while(1)
    {
    	vTaskDelay(1000 / portTICK_PERIOD_MS);;
    	printf("Adc1 : %d\n", pot_getCurrentMeasurement(POTENTIOMETER_1));
    	printf("Adc2 : %d\n", pot_getCurrentMeasurement(POTENTIOMETER_2));
    	printf("Adc3 : %d\n", pot_getCurrentMeasurement(POTENTIOMETER_3));
    	printf("Adc4 : %d\n", pot_getCurrentMeasurement(POTENTIOMETER_4));


    }
#endif
}


void hw_task(void *pvParameter)
{
  while(1)
  {
	  vTaskDelay(100 / portTICK_PERIOD_MS);
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
