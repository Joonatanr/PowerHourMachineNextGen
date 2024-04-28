/*
 * Fonts.h
 *
 *  Created on: 6. dets 2023
 *      Author: JRE
 */

#ifndef LOGIC_FONTS_FONTS_H_
#define LOGIC_FONTS_FONTS_H_

#include "typedefs.h"

 typedef struct
 {
     const uint16_t *data;
     uint16_t width;
     uint16_t height;
     uint8_t dataSize;
 } tImage;

 typedef struct
 {
     long int code;
     const tImage *image;

 } tChar;

 typedef struct
 {
     int length;
     const tChar *chars;
 } tFont;


typedef enum
{
    FONT_ARIAL_12,
    FONT_ARIAL_14_BOLD,
    FONT_ARIAL_16_BOLD,
	FONT_ARIAL_24_BOLD,

    FONT_TNR_HUGE_NUMBERS,

    NUMBER_OF_FONTS
} FontType_t;

/* Legacy conversion */
#define FONT_TINY_FONT		FONT_ARIAL_12
#define FONT_SMALL_FONT		FONT_ARIAL_14_BOLD
#define FONT_MEDIUM_FONT 	FONT_ARIAL_16_BOLD
#define FONT_LARGE_FONT 	FONT_ARIAL_24_BOLD
#define FONT_HUGE_FONT		FONT_TNR_HUGE_NUMBERS

extern const tFont Arial14Bold;
extern const tFont FontArial24Bold;
extern const tFont FontLargeText;
extern const tFont ClockFont;
extern const tFont Arial12;


extern U8 font_getCharWidth(char asc, FontType_t font);
extern U8 font_getFontHeight(FontType_t font);
extern const tFont * font_get_font_ptr(FontType_t font);


#endif /* LOGIC_FONTS_FONTS_H_ */
