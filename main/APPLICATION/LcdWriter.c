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
Private U16 priv_char_buf[4000u];

/* Variables for using the custom printf function */
Private U16 priv_current_line = 0;
Private U16 priv_current_line_distance = 4;
Private FontType_t priv_current_printf_font = FONT_ARIAL_12;
Private U16 priv_current_printf_x = 0;

Private char priv_int_buffer[20];

/***************************** Private function forward declarations  *****************************/

Private U16 getPrintfYCoord(void);


/***************************** Public functions  **************************************************/
Public U16 LcdWriter_drawChar(char c, int x, int y, FontType_t font)
{
    U16 res = 0u;
    U16 index;

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
            display_drawImage(x, y, font_ptr->chars[index].image->width,
                              font_ptr->chars[index].image->height,
                              font_ptr->chars[index].image->data);

            res = font_ptr->chars[index].image->width;
        }
    }

    return res;
}

Public U16 LcdWriter_drawCharColored(char c, int x, int y, FontType_t font, U16 foreground, U16 background)
{
    U16 res = 0u;
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

Public U16 LcdWriter_drawColoredString(const char * str, int x, int y, FontType_t font, U16 foreground, U16 background)
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

    return LcdWriter_getStringWidth(str, font);
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

Public void LcdWriter_resetLine(void)
{
	priv_current_line = 0;
	priv_current_printf_x = 0;
}

Public void LcdWriter_setFont(FontType_t font)
{
	priv_current_printf_font = font;
}

Public void LcdWriter_setLineDistance(U16 distance)
{
	priv_current_line_distance = distance;
}

Public void LcdWriter_setLine(U16 line)
{
	priv_current_line = line;
}

Public void LcdWriter_printf(const char* format, ...)
{
    va_list valist;
    va_start(valist, format);
    int i;
    char *s;


    while(*format)
    {
    	if(*format == '\n')
    	{
    		priv_current_line++;
    		priv_current_printf_x = 0;
    		format++;
    	}
    	else if(*format != '%')
        {
        	priv_current_printf_x += LcdWriter_drawCharColored(*format, priv_current_printf_x, getPrintfYCoord(), priv_current_printf_font, disp_text_color, disp_background_color);
        	format++;
        }
        else
        {
            switch(*++format)
            {
            case 'c':
                i = va_arg(valist, int);
                priv_current_printf_x += LcdWriter_drawCharColored((char)i, priv_current_printf_x, getPrintfYCoord(), priv_current_printf_font, disp_text_color, disp_background_color);
                break;
            case 's':
                s = va_arg(valist, char*);
                /* TODO */
                priv_current_printf_x += LcdWriter_drawColoredString(s, priv_current_printf_x, getPrintfYCoord(), priv_current_printf_font, disp_text_color, disp_background_color);
                break;
            case 'd':
            case 'i':
            	/* TODO */
                i = va_arg(valist, int);
                long2string(i, priv_int_buffer);
                priv_current_printf_x += LcdWriter_drawColoredString(priv_int_buffer, priv_current_printf_x, getPrintfYCoord(), priv_current_printf_font, disp_text_color, disp_background_color);
                break;
            }

            ++format;
        }
    }
}


Private U16 getPrintfYCoord(void)
{
	U16 printLineWidth = font_getFontHeight(priv_current_printf_font);
	printLineWidth += priv_current_line_distance;

	return printLineWidth * priv_current_line;
}

