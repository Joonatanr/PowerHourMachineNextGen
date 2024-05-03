/*
 * LcdWriter.h
 *
 *  Created on: 7. dets 2023
 *      Author: JRE
 */

#ifndef LOGIC_LCDWRITER_H_
#define LOGIC_LCDWRITER_H_


#include "typedefs.h"
#include "Fonts/Fonts.h"
#include "ColorScheme.h"
#include <stdarg.h>

extern U16 LcdWriter_drawChar(char c, int x, int y, FontType_t font);
extern void LcdWriter_drawString(char * str, int x, int y, FontType_t font);
extern U16 LcdWriter_drawCharColored(char c, int x, int y, FontType_t font, U16 foreground, U16 background);
extern U16 LcdWriter_drawColoredString(const char * str, int x, int y, FontType_t font, U16 foreground, U16 background);
extern void LcdWriter_drawStringCenter(const char * str, U8 centerPoint, U8 yloc, FontType_t font, U16 foreground, U16 background);

extern void display_drawStringCenter(const char * str, U8 centerPoint, U8 yloc, FontType_t font, Boolean isInverted);
extern void display_drawString(const char * str, U8 x, U8 y, FontType_t font, Boolean isInverted);
extern U16 LcdWriter_getStringWidth(const char * str, FontType_t font);

/* printf API */

extern void LcdWriter_resetLine(void);
extern void LcdWriter_setFont(FontType_t font);
extern void LcdWriter_setLineDistance(U16 distance);
extern void LcdWriter_setLine(U16 line);
extern void LcdWriter_printf(const char* format, ...);

#endif /* LOGIC_LCDWRITER_H_ */
