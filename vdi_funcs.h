// Prototypes of public VDI functions

#ifndef VDI_FUNCS
#define VDI_FUNCS

#include <stdint.h>
#include "vdi.h"
#include "attribute.h"
#include "helper_structs.h"
#include "workstation.h"

void vs_clip(uint16_t handle, uint16_t clip_flag, vdi_rectangle_t *rect);
void vr_recfl(uint16_t handle, vdi_rectangle_t *rect);
void v_pline(uint16_t handle, uint16_t count, const vdi_point_t *points);
void v_bar(uint16_t handle, vdi_rectangle_t *rect);

// line.c
void polyline(workstation_t *wk, const vdi_point_t *point, int count, int16_t color);

#endif