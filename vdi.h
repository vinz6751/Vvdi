#ifndef VDI_H
#define VDI_H

#include <stdint.h>


typedef struct {
    uint16_t x;
    uint16_t y;
} vdi_point_t;


// Clipping settings for a workstation
typedef struct {
    vdi_point_t p1;
    vdi_point_t p2;
    bool     enabled;
} clip_settings_t;


// VDI parameter block for callers.
// typedef struct
// {
//     uint16_t *contrl;
//     uint16_t *intin;
//     uint16_t *ptsin;
//     uint16_t *intout;
//     uint16_t *ptsout;
// } vdi_parameter_block_t;


#endif
