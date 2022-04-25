#ifndef UTILS_H
#define UTILS_H

// Useful macros
#ifndef R8
    #define R8(x) *((volatile int8_t * const)(x))
#endif
#ifndef R16
    #define R16(x) *((volatile uint16_t * const)(x))
#endif
#ifndef R32
    #define R32(x) *((volatile uint32_t * const)(x))
#endif

#define MIN(a,b) (a < b ? a : b)
#define MAX(a,b) (a > b ? a : b)

#endif
