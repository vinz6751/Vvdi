#ifndef TEXT_H
#define TEXT_H

#include <stdint.h>

/* See TOS.hyp , these effects are publicly documented */
#define TEXT_FONT_FX_NORMAL     0x00
#define TEXT_FONT_FX_THICKENED  0x01
#define TEXT_FONT_FX_LIGHTENED  0x02
#define TEXT_FONT_FX_SKEWED     0x04
#define TEXT_FONT_FX_UNDERLINED 0x08
#define TEXT_FONT_FX_OUTLINED   0x10
#define TEXT_FONT_FX_SHADOWED   0x20

#define TEXT_FONT_SUPPORTED_EFFECTS (TEXT_FONT_FX_NORMAL)

int16_t vst_color(int16_t handle, int16_t color_index);
void vst_height(int16_t handle, int16_t height, int16_t *char_width, int16_t *char_height, int16_t *cell_width, int16_t *cell_height);
void v_gtext(int16_t handle, vdi_point_t xy, uint16_t *string);

#endif