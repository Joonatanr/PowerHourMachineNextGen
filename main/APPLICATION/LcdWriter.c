/*
 * LcdWriter.c
 *
 *  Created on: 7. dets 2023
 *      Author: JRE
 */

#include "LcdWriter.h"
#include "display.h"
#include "misc.h"


/* Distance between the y coordinates of two consecutive lines in a string. */
Private U16 priv_line_distance = 18u;
Private U16 priv_char_buf[1320u * 2u];



/***************************** Public functions  **************************************************/
Public U8 LcdWriter_drawChar(char c, int x, int y, FontType_t font)
{
    U8 res = 0u;

    if(c >= 0x20u)
    {
        U16 index = c - 0x20;

        if (font < NUMBER_OF_FONTS)
        {
            const tFont * font_ptr = font_get_font_ptr(font);
            display_drawImage(x, y, font_ptr->chars[index].image->width,
                              font_ptr->chars[index].image->height,
                              font_ptr->chars[index].image->data);

            res = font_ptr->chars[index].image->width;
        }
    }

    return res;
}

Public U8 LcdWriter_drawCharColored(char c, int x, int y, FontType_t font, U16 foreground, U16 background)
{
    U8 res = 0u;
    U32 ix;
    U16 index;
    U16 total_size;

    if(c >= 0x20u)
    {
        if (font == FONT_TNR_HUGE_NUMBERS)
        {
           /* Special case... */
           if (c >= '0' && c <= ':')
           {
               index = c- '0';
           }
           else
           {
               return 0;
           }
        }
        else
        {
            index = c - 0x20;
        }

        if (font < NUMBER_OF_FONTS)
        {
            const tFont * font_ptr = font_get_font_ptr(font);

            if (( (x + font_getCharWidth(c, font)) >= DISPLAY_WIDTH) || ((y + font_getFontHeight(font)) >= DISPLAY_HEIGHT))
            {
                return 0u;
            }

            total_size = font_ptr->chars[index].image->height * font_ptr->chars[index].image->width * sizeof(U16);

            memcpy(priv_char_buf, font_ptr->chars[index].image->data, font_ptr->chars[index].image->height * font_ptr->chars[index].image->width * sizeof(U16));

            for (ix = 0u; ix < total_size; ix++)
            {
                if (priv_char_buf[ix] == 0xffffu)
                {
                    priv_char_buf[ix] = background;
                }
                else
                {
                    priv_char_buf[ix] = foreground;
                }
            }


            display_drawImage(x, y, font_ptr->chars[index].image->width,
                              font_ptr->chars[index].image->height,
                              priv_char_buf);

            res = font_ptr->chars[index].image->width;
        }
    }

    return res;
}


Public void LcdWriter_drawString(char * str, int x, int y, FontType_t font)
{
    U16 xCoord = x;
    U16 yCoord = y;
    char * str_ptr = str;

    while(*str_ptr)
    {
        if (*str_ptr == '\n')
        {
            /* Line break. */
            xCoord = x;
            yCoord += priv_line_distance;
            str_ptr++;
        }
        else
        {
            xCoord += LcdWriter_drawChar(*str_ptr, xCoord, yCoord, font);
            str_ptr++;
        }
    }
}

Public void LcdWriter_drawColoredString(const char * str, int x, int y, FontType_t font, U16 foreground, U16 background)
{
    U16 xCoord = x;
    U16 yCoord = y;
    const char * str_ptr = str;

    while(*str_ptr)
    {
        if (*str_ptr == '\n')
        {
            /* Line break. */
            xCoord = x;
            yCoord += priv_line_distance;
            str_ptr++;
        }
        else
        {
            xCoord += LcdWriter_drawCharColored(*str_ptr, xCoord, yCoord, font, foreground, background);
            str_ptr++;
        }
    }
}

Public U16 LcdWriter_getStringWidth(const char * str, FontType_t font)
{
    const char *ps = str;
    U8 width = 0u;

    while(*ps)
    {
        if (*ps == '\n')
        {
            break;
        }

        width += font_getCharWidth(*ps, font);
        width++; //Account for one bit in between chars.
        ps++;
    }

    return width;
}

//Draws string around the centerPoint.
Public void LcdWriter_drawStringCenter(const char * str, U8 centerPoint, U8 yloc, FontType_t font, U16 foreground, U16 background)
{
    U16 str_width; //String width in bits.
    U8 begin_point;
    if(str != NULL)
    {
        str_width = LcdWriter_getStringWidth(str, font);
        str_width = str_width >> 1u; //Divide with 2.

        if (str_width > centerPoint)
        {
            //String is too large to fit to display anyway.
            begin_point = 0u;
        }
        else
        {
            begin_point = centerPoint - str_width;
        }

        LcdWriter_drawColoredString(str, begin_point, yloc, font, foreground, background);
    }
}


/* Retain name for legacy purposes... */
Public void display_drawStringCenter(const char * str, U8 centerPoint, U8 yloc, FontType_t font, Boolean isInverted)
{
    if (isInverted)
    {
        LcdWriter_drawStringCenter(str, centerPoint, yloc, font, disp_background_color, disp_text_color);
    }
    else
    {
        LcdWriter_drawStringCenter(str, centerPoint, yloc, font, disp_text_color, disp_background_color);
    }
}


Public void display_drawString(const char * str, U8 x, U8 y, FontType_t font, Boolean isInverted)
{
    if (isInverted)
    {
        LcdWriter_drawColoredString(str, x, y, font, disp_background_color, disp_text_color);
    }
    else
    {
        LcdWriter_drawColoredString(str, x, y, font, disp_text_color, disp_background_color);
    }
}

