/*
 * hardware.h
 *
 *  Created on: Apr 25, 2024
 *      Author: JRE
 */

#ifndef MAIN_HARDWARE_H_
#define MAIN_HARDWARE_H_

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

void hardware_init(void);
void hardware_main(void);

uint16_t hardware_get_pot_value(void);

#endif /* MAIN_HARDWARE_H_ */
