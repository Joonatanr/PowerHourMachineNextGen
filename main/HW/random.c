/*
 * random.c
 *
 *  Created on: Apr 27, 2024
 *      Author: JRE
 */

#include "random.h"
#include "systimer.h"

Private U16 priv_seed = 0u;

Public void regenerate_random_number_seed(void)
{
    priv_seed = systimer2_getTimer();
    /* We initialize the pseudo random number generator. */
    srand(priv_seed);
}


Public U16 generate_random_number_rng(U16 min, U16 max)
{
    U16 range = max - min;
    U16 res = 0u;

    if (range > 0u)
    {
        res = generate_random_number(range);
        res += min;
    }

    return res;
}

Public U16 generate_random_number(U16 max)
{
    U16 res;
    if (priv_seed == 0u)
    {
        regenerate_random_number_seed();
    }

    /* Max might actually be legitimately 0 in some calculations. */
    if (max == 0u)
    {
        res = 0u;
    }
    else
    {
        res =  rand() % (max + 1u);
    }

#ifdef RANDOM_SEED_DEBUG
        printf("Random seed : %d, Random Number : %d, (%d max)\n", priv_seed, res, max);
#endif

    return res;
}
