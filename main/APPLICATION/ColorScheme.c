/*
 * ColorScheme.c
 *
 *  Created on: 27 Dec 2023
 *      Author: Joonatan
 */

#include "ColorScheme.h"
#include "Configuration.h"
#include "display.h"


/***************************** Public variables  **************************************************/

U16 disp_background_color = COLOR_BLACK;
U16 disp_text_color = COLOR_GREEN;
U16 disp_highlight_color = COLOR_CYAN;
U16 disp_ph_prompt_text_color = COLOR_RED;

/***************************** Private variables  *************************************************/

Private U16 priv_selected_color_scheme_index = 0u;

/***************************** Public functions  **************************************************/

Public void ColorScheme_start(void)
{
    setSelectedColorSchemeIndex(configuration_getItem(CONFIG_ITEM_COLOR_SCHEME));
}

Public U16 getSelectedColorScheme(void)
{
    return priv_selected_color_scheme_index;
}


Public void setSelectedColorSchemeIndex(U16 index)
{
    priv_selected_color_scheme_index = index;

    switch(priv_selected_color_scheme_index)
    {
        case 3:
            disp_background_color = COLOR_BLACK;
            disp_text_color = COLOR_CYAN;
            disp_highlight_color = COLOR_RED;
            disp_ph_prompt_text_color = COLOR_MAGENTA;
            break;
        case 2:
            disp_background_color = COLOR_WHITE;
            disp_text_color = COLOR_BLACK;
            disp_highlight_color = COLOR_BLUE;
            disp_ph_prompt_text_color = COLOR_RED;
            break;
        case 1:
            disp_background_color = COLOR_BLUE;
            disp_text_color = COLOR_WHITE;
            disp_highlight_color = COLOR_CYAN;
            disp_ph_prompt_text_color = COLOR_RED;
            break;
        case 0:
        default:
            disp_background_color = COLOR_BLACK;
            disp_text_color = COLOR_GREEN;
            disp_highlight_color = COLOR_CYAN;
            disp_ph_prompt_text_color = COLOR_RED;
            break;
    }

    configuration_setItem(index, CONFIG_ITEM_COLOR_SCHEME);
}

