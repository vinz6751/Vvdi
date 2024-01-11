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

#define MAKE_UINT16(hi,lo) (((uint16_t)(uint8_t)(hi) << 8) | (uint8_t)(lo))
#define MAKE_UINT32(hi,lo) (((uint32_t)(uint16_t)(hi) << 16) | (uint16_t)(lo))

#define HIGH32(x) (((uint32_t)x) >> 16)
#define LOW32(x)  (((uint32_t)x) & 0xffff)

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


#define swap16(a)                         \
  __asm__ volatile                        \
  ("ror   #8,%0"                          \
  : "=d"(a)          /* outputs */        \
  : "0"(a)           /* inputs  */        \
  : "cc"             /* clobbered */      \
  )


#define swap32(a)                         \
  __asm__ volatile                        \
  ("ror   #8,%0\n\t"                      \
   "swap  %0\n\t"                         \
   "ror   #8,%0"                          \
  : "=d"(a)          /* outputs */        \
  : "0"(a)           /* inputs  */        \
  : "cc"             /* clobbered */      \
  )


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

int16_t util_read_file_to_memory(const char *filename, void **file, int32_t *size);
char get_boot_drive();

#endif