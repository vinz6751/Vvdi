#ifndef MATH_H
#define MATH_H

#include <stdint.h>

int16_t isqrt(uint32_t x);
int16_t Icos(int16_t angle);
int16_t Isin(int16_t angle);

// Perform int16_t multiply/divide with rounding
int16_t mul_div_round(int16_t mult1, int16_t mult2, int16_t divisor);
int32_t muls(int16_t m1, int16_t m2);

#endif