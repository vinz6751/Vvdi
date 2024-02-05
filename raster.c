/* Raster functions */

#include <stdint.h>
#include "vdi.h"

void v_get_pixel(int16_t handle, int16_t x, int16_t y, int16_t *pel, int16_t *index ) {
    vdi_point_t pts = { x, y };
    workstation[handle].driver->get_pixels(&pts, 1, pel, index);
}
