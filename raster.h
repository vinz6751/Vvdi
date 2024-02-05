#ifndef RASTER_H
#define RASTER_H

#include "vdi.h"

void vr_trnfm(const MFDB *src_mfdb, MFDB *dst_mfdb);
void v_get_pixel(int16_t handle, int16_t x, int16_t y, int16_t *pel, int16_t *index );

#endif