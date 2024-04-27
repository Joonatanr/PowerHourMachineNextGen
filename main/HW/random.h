/*
 * random.h
 *
 *  Created on: Apr 27, 2024
 *      Author: JRE
 */

#ifndef MAIN_HW_RANDOM_H_
#define MAIN_HW_RANDOM_H_


#include "typedefs.h"

extern void regenerate_random_number_seed(void);
extern U16 generate_random_number_rng(U16 min, U16 max);
extern U16 generate_random_number(U16 max);

#endif /* MAIN_HW_RANDOM_H_ */
