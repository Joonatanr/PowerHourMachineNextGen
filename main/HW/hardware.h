/*
 * hardware.h
 *
 *  Created on: Apr 25, 2024
 *      Author: JRE
 */

#ifndef MAIN_HARDWARE_H_
#define MAIN_HARDWARE_H_

#include "typedefs.h"

void hardware_init(void);
void hardware_main(void);

extern uint16_t hardware_get_pot_value(uint8_t channel);

#endif /* MAIN_HARDWARE_H_ */
