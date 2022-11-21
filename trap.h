// VDI trap interface
#ifndef TRAP_H
#define TRAP_H

#include <stdint.h>
#include "helper_structs.h"

void trap_install(void);
void trap_uninstall(void);

// contrl array of the VDI parameter block.
typedef union {
    struct {
        uint16_t opcode;
        uint16_t ptsin_count;
        uint16_t ptsout_count;
        uint16_t intin_count;
        uint16_t intout_count;
        uint16_t subopcode;
        uint16_t wkid;
    };
    uint16_t word[12];
} vdi_pb_contrl_t;

// VDI parameter block for trap interface.
typedef struct
{
    vdi_pb_contrl_t *contrl;
    uint16_t *intin;
    union {
        uint16_t *words;
        vdi_point_t *pts;
        vdi_rectangle_t *rect;
    } ptsin;
    uint16_t *intout;
    union {
        uint16_t *words;
        vdi_point_t *pts;
    } ptsout;
} vdi_pb_t;

#endif