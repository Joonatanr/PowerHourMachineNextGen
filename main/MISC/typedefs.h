/*
 * typedefs.h
 *
 *  Created on: 21.02.2016
 *      Author: Joonatan
 */

#ifndef TYPEDEFS_H_
#define TYPEDEFS_H_

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

typedef unsigned char   U8;
typedef unsigned short  U16;
typedef unsigned int    U32;

typedef struct
{
    U16 x;
    U16 y;
} Point;

typedef struct
{
    U16 height;
    U16 width;
} Size;

typedef struct
{
    Point location;
    Size  size;
} Rectangle;

typedef union
{
    struct
    {
        U8 msb;
        U8 lsb;
    } bytes;
    U16 value;
} SplitU16;

typedef enum { FALSE, TRUE } Boolean;

#if 0
#ifndef bool
#define bool Boolean
#endif

#ifndef true
#define true TRUE
#endif

#ifndef false
#define false FALSE
#endif
#endif

//////// masking
#define ISBIT(p,b)    (p & b)
#define SETBIT(p,b)   p |= b
#define CLRBIT(p,b)   p &= ~b

#define BIT_0  0x01u
#define BIT_1  0x02u
#define BIT_2  0x04u
#define BIT_3  0x08u
#define BIT_4  0x10u
#define BIT_5  0x20u
#define BIT_6  0x40u
#define BIT_7  0x80u

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#define Private static
#define Public

#define NUMBER_OF_ITEMS(arr) (sizeof((arr)) / sizeof((arr)[0]))

#endif /* TYPEDEFS_H_ */
