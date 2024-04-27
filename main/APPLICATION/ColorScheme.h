/*
 * ColorScheme.h
 *
 *  Created on: 27 Dec 2023
 *      Author: Joonatan
 */

#ifndef LOGIC_COLORSCHEME_H_
#define LOGIC_COLORSCHEME_H_


#include "typedefs.h"

/* Customizable colors. */
extern U16 disp_background_color;
extern U16 disp_text_color;
extern U16 disp_highlight_color;
extern U16 disp_ph_prompt_text_color;

extern void ColorScheme_start(void);
extern U16 getSelectedColorScheme(void);
extern void setSelectedColorSchemeIndex(U16 index);


#endif /* LOGIC_COLORSCHEME_H_ */
