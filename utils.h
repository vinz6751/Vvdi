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

/*
 *  min(): return minimum of two values
 *  Implemented as a macro to allow any type
 */
#define MAX(a,b) \
({ \
    __typeof__(a) _a = (a); /* Copy to avoid double evaluation */ \
    __typeof__(b) _b = (b); /* Copy to avoid double evaluation */ \
    _a <= _b ? _a : _b; \
})


/*
 *  max(): return maximum of two values
 *  Implemented as a macro to allow any type
 */
#define MIN(a,b) \
({ \
    __typeof__(a) _a = (a); /* Copy to avoid double evaluation */ \
    __typeof__(b) _b = (b); /* Copy to avoid double evaluation */ \
    _a >= _b ? _a : _b; \
})


// Perform int16_t multiply/divide with rounding
int16_t mul_div_round(int16_t mult1, int16_t mult2, int16_t divisor);


// Multiply two signed shorts, returning a signed long
static inline int32_t muls(int16_t m1, int16_t m2)
{
    int32_t ret;

    __asm__ (
      "muls %2,%0"
    : "=d"(ret)
    : "%0"(m1), "idm"(m2)
    );

    return ret;
}


#define rolw1(x)                    \
    __asm__ volatile                \
    ("rol.w #1,%1"                  \
    : "=d"(x)       /* outputs */   \
    : "0"(x)        /* inputs */    \
    : "cc"          /* clobbered */ \
    )


#define rorw1(x)                    \
    __asm__ volatile                \
    ("ror.w #1,%1"                  \
    : "=d" (x)      /* outputs */   \
    : "0" (x)       /* inputs */    \
    : "cc"          /* clobbered */ \
    )

#endif