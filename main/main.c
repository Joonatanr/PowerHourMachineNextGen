#include "typedefs.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "hardware.h"
#include "buttons.h"
#include "pot.h"
#include "display.h"
#include "SdCardHandler.h"
#include "Configuration.h"

/* For debugging */
Private led_strip_handle_t led_strip;
Private const char *TAG = "PH Main";
#define BLINK_GPIO 48u


Private void configure_led(void);
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

	/* Setup the TFT display. */
	display_init();

	vTaskDelay(1000 / portTICK_PERIOD_MS);

    /* Initialize the SD Card reader*/
	sdCard_init();

    //Set up the configuration
    configuration_start();

    ColorScheme_start();

#if 0
    backlight_set_level(configuration_getItem(CONFIG_ITEM_BRIGHTNESS));

    /* Initialize the main scheduler. */
    Scheduler_initTasks();

    timer_delay_msec(100);

    //Start all scheduler task
    Scheduler_StartTasks();
    timer_delay_msec(100);

#ifdef POT_TEST
    timer_delay_msec(200u);
    pot_test();
#endif

#ifdef STR_TEST
    str_test();
    timer_delay_msec(10000);
#endif

#ifdef STR_LENGTH_TEST
    SpecialTask_StringLengthSanityTest();
    timer_delay_msec(5000);
#endif

    //We show the initial start screen for a while.
    showStartScreen();

    /* We pass control over to the menu handler. */
    menu_enterMenu(&StartMenu, TRUE);

    /* Sleeping when not in use */
	for(;;)
	{
	    /* Trap CPU... */
	    if (priv_50msec_flag == 1u)
	    {
	        priv_50msec_flag = 0u;
	        timer_50msec_callback();
	    }
	}
#endif

	while(1)
	{
		vTaskDelay(50 / portTICK_PERIOD_MS);
		/* TODO : Main thread runs here. */
	}
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

/* Onboard RGB LED is used for debugging purposes. */
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
