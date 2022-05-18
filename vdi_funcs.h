// Prototypes of public VDI functions

#ifndef VDI_FUNCS
#define VDI_FUNCS

#include <stdint.h>
#include "helper_structs.h"

void v_opnwk(const uint16_t *input, uint16_t *handle, uint16_t *output);
void v_clswk(uint16_t handle);
void v_clrwk(uint16_t handle);
void vs_clip(uint16_t handle, uint16_t clip_flag, const vdi_rectangle_t *rect);
void vs_color(int16_t handle, int16_t index, int16_t *rgb_in);
int16_t vsl_color(int16_t handle, int16_t index);
int16_t vsf_color(int16_t handle, int16_t index);
int16_t vsf_perimeter(int16_t handle, int16_t enable);
uint16_t vsf_style(uint16_t handle, uint16_t style);
uint16_t vsf_interior(uint16_t handle, uint16_t style);
void vqf_attributes(uint16_t handle, uint16_t *output);
uint16_t vswr_mode(uint16_t handle, uint16_t mode);
void vr_recfl(uint16_t handle, vdi_point_t *pts);

#endif