/*
 * ScrollBar.c
 *
 *  Created on: Mar 21, 2018
 *      Author: Joonatan
 */

#include "Bargraph.h"
#include "buttons.h"
#include "backlight.h"
#include "display.h"
#include "LcdWriter.h"
#include "MISC/misc.h"
#include "PowerHourMain.h"

//#include <LOGIC/SnakeGame/SnakeMain.h>

#define BARGRAPH_BEGIN_X    31u
#define BARGRAPH_WIDTH     100u
#define BARGRAPH_HEIGHT      8u
#define BARGRAPH_OFFSET_Y   80u

#define UP_ARROW_OFFSET_Y   30u
#define DOWN_ARROW_OFFSET_Y 60u

#define NUMBER_OFFSET_Y     40u




/******************/

/* Lets define all available scrollbars here as public variables. */
Public Bargraph_T BRIGHTNESS_BARGRAPH =
{
     .max_value = 100u,
     .min_value = 0u,
     .increment = 5u,
     .value = 60u,
     .parent = NULL,
     .text = "Brightness",
     .value_changed = backlight_set_level,
     .config_item = CONFIG_ITEM_BRIGHTNESS,
};

Public Bargraph_T TASK_FREQUENCY_BARGRAPH =
{
     .max_value = 10u,
     .min_value = 1u,
     .increment = 1u,
     .value = 2u,
     .parent = NULL,
     .text = "Task Frequency (minutes)",
     .value_changed = powerHour_setTaskFrequency,
     .config_item = CONFIG_ITEM_TASK_FREQ,
};

/*******************/

Private Bargraph_T * priv_active_bar;
Private char priv_buf[10];


Private const U16 priv_up_arrow_bmp[5][9] =
{
     { 0x0000u, 0x0000u, 0x0000u, 0x0000u, 0xffffu, 0x0000u, 0x0000u, 0x0000u, 0x0000u },
     { 0x0000u, 0x0000u, 0x0000u, 0xffffu, 0xffffu, 0xffffu, 0x0000u, 0x0000u, 0x0000u },
     { 0x0000u, 0x0000u, 0xffffu, 0xffffu, 0xffffu, 0xffffu, 0xffffu, 0x0000u, 0x0000u },
     { 0x0000u, 0xffffu, 0xffffu, 0xffffu, 0xffffu, 0xffffu, 0xffffu, 0xffffu, 0x0000u },
     { 0xffffu, 0xffffu, 0xffffu, 0xffffu, 0xffffu, 0xffffu, 0xffffu, 0xffffu, 0xffffu },
};


Private const U16 priv_down_arrow_bmp[5][9] =
{
     { 0xffffu, 0xffffu, 0xffffu, 0xffffu, 0xffffu, 0xffffu, 0xffffu, 0xffffu, 0xffffu },
     { 0x0000u, 0xffffu, 0xffffu, 0xffffu, 0xffffu, 0xffffu, 0xffffu, 0xffffu, 0x0000u },
     { 0x0000u, 0x0000u, 0xffffu, 0xffffu, 0xffffu, 0xffffu, 0xffffu, 0x0000u, 0x0000u },
     { 0x0000u, 0x0000u, 0x0000u, 0xffffu, 0xffffu, 0xffffu, 0x0000u, 0x0000u, 0x0000u },
     { 0x0000u, 0x0000u, 0x0000u, 0x0000u, 0xffffu, 0x0000u, 0x0000u, 0x0000u, 0x0000u },
};



/***************************** Private function forward declarations  ******************************/

Private void drawBarGraph(void);
Private void drawBackGround(void);

Private void handleButtonUp(void);
Private void handleButtonDown(void);
Private void handleButtonAck(void);

Private void updateBargraph(void);


/***************************** Public function definitions  ******************************/


/* Called when a scrollbar becomes active. */
Public void enterBarGraph(Bargraph_T * bar)
{
    priv_active_bar = bar;

    buttons_subscribeListener(UP_BUTTON, handleButtonUp);
    buttons_subscribeListener(DOWN_BUTTON, handleButtonDown);
    buttons_subscribeListener(OK_BUTTON, handleButtonAck);
    buttons_subscribeListener(CANCEL_BUTTON, handleButtonAck);

    //priv_number_box = CreateRectangleAroundCenter((Point){64u, NUMBER_OFFSET_Y }, (Size){ 20u, 20u });

    if (bar->config_item < NUMBER_OF_CONFIG_ITEMS)
    {
        bar->value = configuration_getItem(bar->config_item);
    }

    drawBackGround();
    drawBarGraph();
}


/***************************** Private function definitions  ******************************/

/* Draws whole scrollbar */
Private void drawBarGraph(void)
{
    U16 percentage;
    U8 range = priv_active_bar->max_value - priv_active_bar->min_value;

    percentage = ((priv_active_bar->value - priv_active_bar->min_value) * 100u) / range;

    //Draw the line
    /* We clear it first. */
    //display_fillRectangle(BARGRAPH_BEGIN_X, BARGRAPH_OFFSET_Y , BARGRAPH_HEIGHT, BARGRAPH_WIDTH, PATTERN_WHITE);
    display_fillRectangle(BARGRAPH_BEGIN_X, BARGRAPH_OFFSET_Y, BARGRAPH_WIDTH, BARGRAPH_HEIGHT , COLOR_RED);

    /* Then draw the actual line. */
    display_fillRectangle(BARGRAPH_BEGIN_X, BARGRAPH_OFFSET_Y , percentage, BARGRAPH_HEIGHT , disp_highlight_color);

    //Draw the number.
    long2string(priv_active_bar->value, priv_buf);
    display_drawStringCenter(priv_buf, DISPLAY_CENTER, NUMBER_OFFSET_Y, FONT_MEDIUM_FONT, FALSE);
}


/* Draws the arrows on the scrollbar sides. */
Private void drawBackGround(void)
{
    display_clear();

    //Draw title
    display_drawStringCenter(priv_active_bar->text, DISPLAY_CENTER, 10u, FONT_SMALL_FONT_12 , FALSE);

    //Draw up arrow.
    display_drawBitmapCenter(&priv_up_arrow_bmp[0][0], DISPLAY_CENTER, UP_ARROW_OFFSET_Y, 9u, 5u);

    //Draw down arrow.
    display_drawBitmapCenter(&priv_down_arrow_bmp[0][0], DISPLAY_CENTER, DOWN_ARROW_OFFSET_Y, 9u, 5u);
}


Private void handleButtonUp(void)
{
    if (priv_active_bar->value < priv_active_bar->max_value)
    {
        priv_active_bar->value += priv_active_bar->increment;
    }

    //Update the displayed data.
    updateBargraph();
}

Private void handleButtonDown(void)
{
    if (priv_active_bar->value > priv_active_bar->min_value)
    {
        priv_active_bar->value -= priv_active_bar->increment;
    }

    //Update the displayed data.
    updateBargraph();
}


Private void updateBargraph(void)
{
    if (priv_active_bar->value_changed != NULL)
    {
       priv_active_bar->value_changed(priv_active_bar->value);
    }

    drawBarGraph();
}

Private void handleButtonAck(void)
{
    if (priv_active_bar->config_item < NUMBER_OF_CONFIG_ITEMS)
    {
        configuration_setItem(priv_active_bar->value, priv_active_bar->config_item);
    }

    if (priv_active_bar->parent != NULL)
    {
        buttons_unsubscribeAll();
        menu_enterMenu(priv_active_bar->parent, FALSE);
        priv_active_bar = NULL;
    }
    /* Else, I guess we are stuck here...   */
    /* Maybe should restart, just in case?  */
}
