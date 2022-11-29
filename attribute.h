#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

// Drawing preferences for lines, colors, fill, perimeter etc.

#include <stdint.h>

// Colors ---------------------------------------------------------------------
void vs_color(int16_t handle, int16_t index, int16_t *rgb_in);
int16_t vq_color(int16_t handle, int16_t index, int16_t set_flag, int16_t *rgb);

// Drawing parameters ---------------------------------------------------------
int16_t vsl_type(int16_t handle, int16_t style);
void vsl_udsty(int16_t handle, int16_t user_defined_style);
int16_t vsl_color(int16_t handle, int16_t index);
int16_t vsf_color(int16_t handle, int16_t index);
int16_t vsf_perimeter(int16_t handle, int16_t enable);
uint16_t vsf_style(uint16_t handle, uint16_t style);
uint16_t vsf_interior(uint16_t handle, uint16_t style);
void vqf_attributes(uint16_t handle, uint16_t *output);
uint16_t vswr_mode(uint16_t handle, uint16_t mode);

#endif
