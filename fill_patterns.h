#ifndef FILL_PATTERNS_H
#define FILL_PATTERNS_H

#include <stdint.h>

/* the storage for the used defined fill pattern */
extern uint16_t ROM_UD_PATRN[16];

extern const uint16_t OEMMSKPAT;
extern const uint16_t OEMPAT[128];

extern const uint16_t DITHRMSK;              /* mask off all but four scans */
extern const uint16_t DITHER[32];

extern const uint16_t HAT_0_MSK;
extern const uint16_t HATCH0[48];

extern const uint16_t HAT_1_MSK;
extern const uint16_t HATCH1[96];

extern const uint16_t HOLLOW;
extern const uint16_t SOLID;

#endif