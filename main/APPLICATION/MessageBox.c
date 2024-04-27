/*
 * MessageBox.c
 *
 *  Created on: Sep 4, 2018
 *      Author: Joonatan
 */

#include "MessageBox.h"
#include "display.h"
#include "misc.h"
#include "buttons.h"
#include "Scheduler.h"
#include "LcdWriter.h"

#define MSGBOX_FONT FONT_MEDIUM_FONT
//#define MSGBOX_FONT FONT_SMALL_FONT

#define MSGBOX_MIN_HEIGHT   60u
#define MSGBOX_MIN_WIDTH    100u
#define MSGBOX_MARGIN       10u
#define BUTTON_AREA_HEIGHT  18u

#define MSGBOX_BORDER_WIDTH 3u

#define BUTTON_MARGIN       30u

typedef enum
{
    STATE_IDLE,
    STATE_PENDING,
    STATE_SHOWING,
    NUMBER_OF_MSGBOX_STATES
} MsgBox_State;

typedef enum
{
    TYPE_TEXT,
    TYPE_OK,
    TYPE_OK_CANCEL,
    NUMBER_OF_TYPES
} MsgBox_Type;

/**************************** Private function forward declarations **************************/

Private void drawMessageBox(const char * text, MsgBox_Type type);
Private Size getMessageSize(const char * text);
Private void drawButton(const char * text, U8 xloc);

Private void handleOkPress(void);
Private void handleCancelPress(void);

Private void closeMessageBox(void);


/* Flags for setting up a displayed messagebox. */
Private MsgBox_State            priv_state;
Private MsgBox_Type             priv_type;
Private char                    priv_text[64];
Private U16                     priv_duration_period;
Private MsgBoxResponseHandler   priv_response_handler = NULL;
Private Rectangle priv_msg_box;

/**************************** Public function definitions **************************/

Public void MessageBox_init(void)
{
    priv_state = STATE_IDLE;
}


Public void MessageBox_cyclic100msec(void)
{
    if (priv_duration_period > 0u)
    {
        priv_duration_period--;
    }

    switch(priv_state)
    {
        case STATE_PENDING:
            switch(priv_type)
            {
            case TYPE_TEXT:
                drawMessageBox(priv_text, priv_type);
                break;
            case TYPE_OK:
                buttons_unsubscribeAll();
                buttons_subscribeListener(OK_BUTTON , handleOkPress);

                drawMessageBox(priv_text, priv_type);
                break;
            case TYPE_OK_CANCEL:
                buttons_unsubscribeAll();

                buttons_subscribeListener(OK_BUTTON , handleOkPress);
                buttons_subscribeListener(CANCEL_BUTTON, handleCancelPress);

                drawMessageBox(priv_text, priv_type);
                break;
            default:
                /* Should not happen. */
                break;
            }

            /* We pause the active module until messagebox has been closed. */
            Scheduler_SetActiveApplicationPause(TRUE);
            priv_state = STATE_SHOWING;
            break;

        case STATE_SHOWING:
            if ((priv_type == TYPE_TEXT) && (priv_duration_period == 0u))
            {
                closeMessageBox();
            }
            break;
        case STATE_IDLE:
        default:
            break;
    }
}


/* Sets callback for messagebox response. */
Public void MessageBox_SetResponseHandler(MsgBoxResponseHandler handler)
{
    priv_response_handler = handler;
}


Public void MessageBox_Show(const char * text, U16 period)
{
    strcpy(priv_text, text);
    priv_duration_period = period;
    priv_state = STATE_PENDING;
    priv_type = TYPE_TEXT;
}


Public void MessageBox_ShowWithOk(const char * text)
{
    strcpy(priv_text, text);
    priv_duration_period = 0u;
    priv_state = STATE_PENDING;
    priv_type = TYPE_OK;
}


Public void MessageBox_ShowWithOkCancel(const char * text)
{
    strcpy(priv_text, text);
    priv_duration_period = 0u;
    priv_state = STATE_PENDING;
    priv_type = TYPE_OK_CANCEL;
}

/**************************** Private function definitions **************************/

Private void drawMessageBox(const char * text, MsgBox_Type type)
{
    Size textSize = getMessageSize(text);
    Size rectSize;
    Point rectLocation;

    rectSize.height = MAX(MSGBOX_MIN_HEIGHT, (textSize.height + MSGBOX_MARGIN));
    rectSize.width =  MAX(MSGBOX_MIN_WIDTH,  (textSize.width + MSGBOX_MARGIN));

    if (type == TYPE_OK || type == TYPE_OK_CANCEL)
    {
        rectSize.height += BUTTON_AREA_HEIGHT;
    }

    rectSize.height = MIN(rectSize.height, DISPLAY_HEIGHT);
    rectSize.width = MIN(rectSize.width, DISPLAY_WIDTH);

    rectLocation.x = GET_X_FROM_CENTER(DISPLAY_WIDTH / 2, rectSize.width);
    rectLocation.y = GET_Y_FROM_CENTER(DISPLAY_HEIGHT / 2, rectSize.height);

    priv_msg_box.location = rectLocation;
    priv_msg_box.size = rectSize;

    /* Draw messagebox. */
    /* First set the background */
    display_fillRectangle(rectLocation.x, rectLocation.y, rectSize.width, rectSize.height, disp_background_color);

    /* Then lets set the borders */
    display_drawRectangle(rectLocation.x, rectLocation.y, rectSize.width, rectSize.height, MSGBOX_BORDER_WIDTH, disp_text_color);

    /* Draw the text. */
    display_drawStringCenter(text, DISPLAY_CENTER, rectLocation.y + 20u, MSGBOX_FONT, FALSE);

    /* Draw the buttons */
    switch(type)
    {
        case TYPE_OK:
            drawButton("Ok", DISPLAY_CENTER);
            break;
        case TYPE_OK_CANCEL:
            drawButton("Ok", DISPLAY_CENTER - 31u);
            drawButton("Cancel", DISPLAY_CENTER + 13u);
            break;
        case TYPE_TEXT:
        default:
            break;

    }
}

Private void drawButton(const char * text, U8 xloc)
{
    display_drawStringCenter(text, xloc, (priv_msg_box.location.y + priv_msg_box.size.height) - BUTTON_MARGIN, FONT_ARIAL_16_BOLD, TRUE);
}


/* Returns physical size of the text to be displayed */
Private Size getMessageSize(const char * text)
{
    Size ret;
    U8 number_of_rows = 1u;
    U8 current_row_width = 0u;
    U8 max_row_width = 0u;
    U8 fontsize = font_getFontHeight(MSGBOX_FONT);

    const char *ps = text;
    while (*ps)
    {
        if (*ps == '\n')
        {
          number_of_rows++;
          max_row_width = MAX(current_row_width, max_row_width);
          current_row_width = 0u;
        }
        else
        {
            current_row_width += font_getCharWidth(*ps, MSGBOX_FONT);
            current_row_width++; //Account for one bit in between chars.
        }
        ps++;
    }

    max_row_width = MAX(current_row_width, max_row_width);
    /* Lets try this with 1 row at first, and later test with multiple rows... */
    ret.height = fontsize * number_of_rows;
    ret.width = max_row_width;

    return ret;
}


Private void handleOkPress(void)
{
    closeMessageBox();
    /* Lets call this last, so that the active module can draw something on the display etc... */
    if (priv_response_handler != NULL)
    {
        priv_response_handler(RESPONSE_OK);
    }
    priv_response_handler = NULL;
}

Private void handleCancelPress(void)
{
    closeMessageBox();
    /* Lets call this last, so that the active module can draw something on the display etc... */
    if (priv_response_handler != NULL)
    {
        priv_response_handler(RESPONSE_CANCEL);
    }
    priv_response_handler = NULL;
}


Private void closeMessageBox(void)
{
    /* TODO : Also draw the previous image, so it does not get lost. */

    //buttons_unsubscribeAll();
    Scheduler_SetActiveApplicationPause(FALSE);
    priv_state = STATE_IDLE;
}
