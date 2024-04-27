/*
 * SdCardHandler.h
 *
 *  Created on: 5. dets 2023
 *      Author: JRE
 */

#ifndef LOGIC_SDCARDHANDLER_H_
#define LOGIC_SDCARDHANDLER_H_

#include "typedefs.h"


extern void sdCard_init(void);
extern void sdCard_Read_bmp_file(const char *path, uint16_t * output_buffer);


#endif /* LOGIC_SDCARDHANDLER_H_ */
