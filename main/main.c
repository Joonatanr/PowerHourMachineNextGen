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
#include "backlight.h"
#include "Scheduler.h"
#include "menu.h"
#include "PowerHourMain.h"
#include "BitmapHandler.h"
#include "LcdWriter.h"

/* For debugging */
Private led_strip_handle_t led_strip;
Private const char *TAG = "PH Main";
Private const char priv_version_string[] = "Machine 5.0";
#define BLINK_GPIO 48u


Private void configure_led(void);
void hw_task(void *pvParameter);

Private void showStartScreen(void);
Private void showDedicationText(void);
Private void startGameHandler(void);


#ifdef STR_TEST
Private char priv_test_buf[32];
#endif

/* Settings Menu Items */
Private const MenuItem ColorMenuItemArray[] =
{
     { .text = "Scheme 1",    .Action = MENU_ACTION_SELECT  , .ActionArg.function_set_u16_ptr = setSelectedColorSchemeIndex  },
     { .text = "Scheme 2",    .Action = MENU_ACTION_SELECT  , .ActionArg.function_set_u16_ptr = setSelectedColorSchemeIndex  },
     { .text = "Scheme 3",    .Action = MENU_ACTION_SELECT  , .ActionArg.function_set_u16_ptr = setSelectedColorSchemeIndex  },
     { .text = "Scheme 4",    .Action = MENU_ACTION_SELECT  , .ActionArg.function_set_u16_ptr = setSelectedColorSchemeIndex  },
};

Private const MenuItem BuzzerMenuItemArray[] =
{
     { .text = "Disabled",    .Action = MENU_ACTION_SELECT  , .ActionArg.function_set_u16_ptr = setBuzzerEnable  },
     { .text = "Enabled",   .Action = MENU_ACTION_SELECT    , .ActionArg.function_set_u16_ptr = setBuzzerEnable  },
};

Private SelectionMenu ColorMenu =
{
     .items = ColorMenuItemArray,
     .number_of_items = NUMBER_OF_ITEMS(ColorMenuItemArray),
     .selected_item = 0u,
     .initial_select_func = getSelectedColorScheme,
     .isCheckedMenu = TRUE,
     .isTransparentMenu = FALSE,
};

Private SelectionMenu BuzzerMenu =
{
     .items = BuzzerMenuItemArray,
     .number_of_items = NUMBER_OF_ITEMS(BuzzerMenuItemArray),
     .selected_item = 0u,
     .initial_select_func = isBuzzerEnabled,
     .isCheckedMenu = TRUE,
     .isTransparentMenu = FALSE,
};

Private const MenuItem SettingsMenuItemArray[] =
{
   { .text = "Brightness",    .Action = MENU_ACTION_WIDGET  , .ActionArg.bargraph_ptr = &BRIGHTNESS_BARGRAPH        },
   { .text = "Task frequency",.Action = MENU_ACTION_WIDGET  , .ActionArg.bargraph_ptr = &TASK_FREQUENCY_BARGRAPH    },
   { .text = "Color scheme"  ,.Action = MENU_ACTION_SUBMENU , .ActionArg.subMenu_ptr =  &ColorMenu                  },
   { .text = "Buzzer"        ,.Action = MENU_ACTION_SUBMENU , .ActionArg.subMenu_ptr =  &BuzzerMenu                 }
};

Private SelectionMenu SettingsMenu =
{
     .items = SettingsMenuItemArray,
     .number_of_items = NUMBER_OF_ITEMS(SettingsMenuItemArray),
     .selected_item = 0u,
     .initial_select_func = NULL,
     .isCheckedMenu = FALSE,
     .isTransparentMenu = FALSE,
};


/** Start Menu Items.*/
Private const MenuItem StartMenuItemArray[] =
{
   { .text = "Start Game",  .Action = MENU_ACTION_FUNCTION    , .ActionArg.function_ptr =   startGameHandler        },
   { .text = "Settings",    .Action = MENU_ACTION_SUBMENU     , .ActionArg.subMenu_ptr  =   &SettingsMenu           },
   { .text = "About",       .Action = MENU_ACTION_FUNCTION    , .ActionArg.function_ptr =   &showDedicationText     },
};

Private SelectionMenu StartMenu =
{
     .items = StartMenuItemArray,
     .number_of_items = NUMBER_OF_ITEMS(StartMenuItemArray),
     .selected_item = 0u,
     .initial_select_func = NULL,
     .isCheckedMenu = FALSE,
     .isTransparentMenu = FALSE, /* Just for experimenting, probably will not really use this. */
};

/** End of Start Menu Items. */

uint16_t priv_test_buf[10][15] =
{
		{ COLOR_BLUE, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_BLUE   , COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE },
		{ COLOR_BLUE, COLOR_BLUE, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_BLUE, COLOR_BLUE , COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE},
		{ COLOR_BLUE, COLOR_RED, COLOR_BLUE, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_BLUE, COLOR_RED, COLOR_BLUE , COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE},
		{ COLOR_BLUE, COLOR_RED, COLOR_RED, COLOR_BLUE, COLOR_RED, COLOR_RED, COLOR_BLUE, COLOR_RED, COLOR_RED, COLOR_BLUE , COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE},
		{ COLOR_BLUE, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_BLUE, COLOR_BLUE, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_BLUE , COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE},
		{ COLOR_BLUE, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_BLUE, COLOR_BLUE, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_BLUE , COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE},
		{ COLOR_BLUE, COLOR_RED, COLOR_RED, COLOR_BLUE, COLOR_RED, COLOR_RED, COLOR_BLUE, COLOR_RED, COLOR_RED, COLOR_BLUE , COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE},
		{ COLOR_BLUE, COLOR_RED, COLOR_BLUE, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_BLUE, COLOR_RED, COLOR_BLUE , COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE},
		{ COLOR_BLUE, COLOR_BLUE, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_BLUE, COLOR_BLUE , COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE},
		{ COLOR_BLUE, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_BLUE   , COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE},
};


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

	vTaskDelay(100 / portTICK_PERIOD_MS);

#if 0
	backlight_set_level(100);
	display_fill(COLOR_GREEN);
	display_fillRectangle(0, 0, 40, 40, COLOR_BLUE);
	display_fillRectangle(40, 40, 40, 40, COLOR_ORANGE);
	display_drawImage(80, 80, 15, 10, &priv_test_buf[0][0]);
	LcdWriter_drawChar('A', 0, 0, FONT_ARIAL_12);
	display_flushBuffer(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);

	vTaskDelay(1000 / portTICK_PERIOD_MS);
	display_fill(COLOR_BLUE);
	LcdWriter_drawColoredString("Hello World", 10, 10, FONT_ARIAL_16_BOLD, COLOR_RED, COLOR_GREEN);
	display_flushBufferAll();

	vTaskDelay(1000 / portTICK_PERIOD_MS);
#endif

    /* Initialize the SD Card reader*/
	sdCard_init();

    //Set up the configuration
    configuration_start();

    ColorScheme_start();

    /* Initialize the main scheduler. */
    Scheduler_initTasks();

    vTaskDelay(100 / portTICK_PERIOD_MS);

    //Start all scheduler task
    Scheduler_StartTasks();
    vTaskDelay(100 / portTICK_PERIOD_MS);

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

	TickType_t xLastWakeTime;
	const TickType_t xFrequency = 50u / portTICK_PERIOD_MS;

	xLastWakeTime = xTaskGetTickCount ();

	while(1)
	{
		vTaskDelayUntil( &xLastWakeTime, xFrequency );
		Scheduler_cyclic();
		display_flushBufferAll();
	}
}

Public void returnToMain(void)
{
    Scheduler_StopActiveApplication();
    menu_enterMenu(&StartMenu, FALSE);
}

Private void showStartScreen(void)
{
    /* Load a bitmap and display it on the screen. */
#ifdef DISPLAY_TEST
    display_test();
    timer_delay_msec(6000u);
#endif

    display_fill(disp_background_color);
    display_flushBuffer(0u, 0u, DISPLAY_WIDTH, DISPLAY_HEIGHT);

    /* Lets turn on the backlight here when we actually have something to show already. */
    uint8_t brightness = configuration_getItem(CONFIG_ITEM_BRIGHTNESS);
    backlight_set_level(brightness);

    //BitmapHandler_LoadBitmap("/test.bmp", display_get_frame_buffer());
    //display_flushBuffer(0u, 0u, DISPLAY_WIDTH, DISPLAY_HEIGHT);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
    //LcdWriter_drawChar('0', 10, 10, FONT_TNR_HUGE_NUMBERS);
    LcdWriter_drawStringCenter("Power Hour", (DISPLAY_WIDTH / 2u) + 4u, 80u, FONT_LARGE_FONT, disp_text_color, disp_background_color);
    LcdWriter_drawStringCenter(priv_version_string, (DISPLAY_WIDTH / 2u) + 4u, 110u, FONT_MEDIUM_FONT, disp_text_color, disp_background_color);
    LcdWriter_drawStringCenter("Enginaator Edition", (DISPLAY_WIDTH / 2u) + 4u, 135u, FONT_LARGE_FONT, disp_text_color, disp_background_color);

    display_flushBuffer(0u, 0u, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    vTaskDelay(5000 / portTICK_PERIOD_MS);

}


Private void showDedicationText(void)
{
    /* Turns out we can't do this without making this into a dummy application for some reason. */
    Scheduler_SetActiveApplication(APPLICATION_DEDICATION);
}

/* Starts the main Power Hour game. */
Private void startGameHandler(void)
{
    Scheduler_SetActiveApplication(APPLICATION_POWER_HOUR);
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
