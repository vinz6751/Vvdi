/*
 * fVDI integer sin/cos/sqrt code
 *
 * Copyright 1999/2001/2003, Johan Klockars
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 *
 * The sin/cos part is an optimized version of code with an
 * original copyright as follows.
 */

/*************************************************************************
**       Copyright 1999, Caldera Thin Clients, Inc.                     **
**       This software is licenced under the GNU Public License.        **
**       Please see LICENSE.TXT for further information.                **
**                                                                      **
**                  Historical Copyright                                **
**                                                                      **
**                                                                      **
**                                                                      **
**  Copyright (c) 1987, Digital Research, Inc. All Rights Reserved.     **
**  The Software Code contained in this listing is proprietary to       **
**  Digital Research, Inc., Monterey, California and is covered by U.S. **
**  and other copyright protection.  Unauthorized copying, adaptation,  **
**  distribution, use or display is prohibited and may be subject to    **
**  civil and criminal penalties.  Disclosure to others is prohibited.  **
**  For the terms and conditions of software code use refer to the      **
**  appropriate Digital Research License Agreement.                     **
**                                                                      **
**************************************************************************/

#include <stdint.h>


// Multiply two signed shorts, returning a signed long
int32_t muls(int16_t m1, int16_t m2)
{
    int32_t ret;

    __asm__ (
      "muls %2,%0"
    : "=d"(ret)
    : "%0"(m1), "idm"(m2)
    );

    return ret;
}


/*
 * Integer sine and cosine functions.
 */
#if 0
#include "fvdi.h"
#include "function.h"
#include "relocate.h"
#include "utility.h"
#endif
#define HALFPI  900
#define PI      1800
#define TWOPI   3600

/* Sines of angles 1 - 90 degrees normalized between 0 and 32767. */

static int16_t sin_tbl[92] = {
    0, 572, 1144, 1716, 2286, 2856, 3425, 3993,
    4560, 5126, 5690, 6252, 6813, 7371, 7927, 8481,
    9032, 9580, 10126, 10668, 11207, 11743, 12275, 12803,
    13328, 13848, 14364, 14876, 15383, 15886, 16383, 16876,
    17364, 17846, 18323, 18794, 19260, 19720, 20173, 20621,
    21062, 21497, 21925, 22347, 22762, 23170, 23571, 23964,
    24351, 24730, 25101, 25465, 25821, 26169, 26509, 26841,
    27165, 27481, 27788, 28087, 28377, 28659, 28932, 29196,
    29451, 29697, 29934, 30162, 30381, 30591, 30791, 30982,
    31163, 31335, 31498, 31650, 31794, 31927, 32051, 32165,
    32269, 32364, 32448, 32523, 32587, 32642, 32687, 32722,
    32747, 32762, 32767, 32767
};




/*
 * Returns integer sin between -32767 and 32767.
 * Uses integer lookup table sintable^[].
 * Expects angle in tenths of degree 0 - 3600.
 * Assumes positive angles only.
 */
int16_t Isin(int16_t angle)
{
    int16_t index;
    int16_t remainder;
    int16_t tmpsin;    /* Holder for sin. */
    int16_t half;      /* 0-1 = 1st/2nd, 3rd/4th. */
    int16_t *table;

    half = 0;
    while (angle >= PI)
    {
        half ^= 1;
        angle -= PI;
    }
    if (angle >= HALFPI)
        angle = PI - angle;

    index = angle / 10;
    remainder = angle % 10;
    table = &sin_tbl[index];
    tmpsin = *table++;
    if (remainder)     /* Add interpolation. */
        tmpsin += (int16_t) ((int16_t) (*table - tmpsin) * remainder) / 10;

    return half > 0 ? -tmpsin : tmpsin;
}


/*
 * Return integer cos between -32767 and 32767.
 */
int16_t Icos(int16_t angle)
{
    return Isin(angle + HALFPI);
}


int16_t isqrt(uint32_t x)
{
    uint32_t s1, s2;

    if (x < 2)
        return x;

    s1 = x;
    s2 = 2;
    do {
        s1 /= 2;
        s2 *= 2;
    } while (s1 > s2);

    s2 = (s1 + (s2 / 2)) / 2;

    do {
        s1 = s2;
        s2 = (x / s1 + s1) / 2;
    } while (s1 > s2);

    return (int16_t)s1;
}
