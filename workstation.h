#ifndef WORKSTATION_H
#define WORKSTATION_H

#include <stdint.h>
#include "vdi.h"


extern workstation_t workstation[];

void workstation_init(void);
void v_opnwk(const uint16_t *input, uint16_t *handle, uint16_t *output);
void v_clswk(uint16_t handle);
void v_clrwk(uint16_t handle);

#endif