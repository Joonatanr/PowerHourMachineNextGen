/*
 * BitmapHandler.h
 *
 *  Created on: 6. dets 2023
 *      Author: JRE
 */

#ifndef LOGIC_BITMAPHANDLER_H_
#define LOGIC_BITMAPHANDLER_H_


#include "typedefs.h"

typedef void(*BitmapLoaderCallback)(void);


typedef enum
{
    FILES_MEN,
    FILES_WOMEN,
    FILES_BESTIES,

    FILES_MEN_HOT,
    FILES_WOMEN_HOT,
    FILES_ALL_HOT,

    NUMBER_OF_FILE_CATEGORIES,

    FILES_NONE
} BitmapHandler_FileCategory_t;

extern Boolean BitmapHandler_LoadBitmap(const char * path, U16 * dest);
extern Boolean BitmapHandler_StartCyclicLoad(const char * path, U16 * dest, BitmapLoaderCallback cb);
extern void BitmapHandler_getRandomBitmapForCategory(BitmapHandler_FileCategory_t type, char *dest);

extern void BitmapHandler_init(void);
extern void BitmapHandler_start(void);
extern void BitmapLoaderCyclic100ms(void);

#endif /* LOGIC_BITMAPHANDLER_H_ */
