/*
 * Bitmaps.c
 *
 *  Created on: 14 Dec 2023
 *      Author: Joonatan
 */

#include "Bitmaps.h"
#include "display.h"


Public void drawBitmap(U8 x, U8 y, const tImage * bmp_ptr)
{
    U16 height = bmp_ptr->height;
    U16 width = bmp_ptr->width;
    const U16 * src_ptr = bmp_ptr->data;

    display_drawImage(x, y, width, height, src_ptr);
}



