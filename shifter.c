// Atari shifter driver

#include <stdint.h>
#include "utils.h"
#include "vdi.h"

// Shifter registers
#define PALETTE    (uint16_t*const)0xffff8240L
#define RESOLUTION 0xffff8260L

static screen_info_t screen_info;

static void resolution_has_changed(void);


static void init(void) {
    resolution_has_changed();
}

static void deinit(void) {
}

static void get_screen_info(uint16_t *resx, uint16_t *resy, uint32_t *ncolors, uint16_t *line_length) {
    *resx = screen_info.max_x + 1;
    *resy = screen_info.max_y + 1;
    *ncolors = screen_info.colors;
    *line_length = screen_info.line_length;
}

#define LUT_ADDRESS(lut, n) (uint8_t*)(VICKY_LUT_BASE + lut*0x400 + n*4)

static inline scale1000to7(uint16_t c) {
    // This involves a division so is not really performant on a 68000...
    return c / 142;
}

// Set color on LUT 0, colors are 8 bits each
static void set_color(uint16_t n, uint16_t red, uint16_t green, uint16_t blue) {
    uint16_t * palette = PALETTE;
    uint16_t color;

    // ST
    color = scale1000to7(red) << 8;
    color += scale1000to7(green) << 4;
    color += scale1000to7(blue);
    palette[n] = color;

    // TODO: STE (scale 1000 to 15)
}

static void get_color(uint16_t n, uint16_t *red, uint16_t *green, uint16_t *blue) {
    const uint16_t *palette = PALETTE;
    uint16_t color = palette[n];

    // ST
    *blue = color & 7;
    color >> 4;
    *green = color & 7;
    color >> 4;
    *red = color & 7;
    
    // TODO: STE
}

static screen_info_t screen_info;

static void resolution_has_changed(void) {
    uint32_t res = R8(0xffff8260) & 3;
    
    switch (res) {
        case 0:
            screen_info.max_x = 320-1;
            screen_info.max_y = 200-1;
            screen_info.colors = 16;
            screen_info.bitplanes = 4;
            screen_info.line_length = 32000/200;
            break;
        case 1:
            screen_info.max_x = 640-1;
            screen_info.max_y = 200-1;
            screen_info.colors = 4;
            screen_info.line_length = 32000/200;
            break;
        case 2: // Not supported on the U
            screen_info.max_x = 640-1;
            screen_info.max_y = 400-1;
            screen_info.colors = 1;
            screen_info.line_length = 32000/400;
            break;
    }
}

static void set_pixels(const vdi_point_t *pts, uint16_t n, uint16_t color) {
    uint16_t *fb = (uint16_t*)R32(v_bas_ad);
    vdi_point_t *pt;
    int i;

    while (n++) {
        pt = (vdi_point_t *)pts++;
        uint16_t *plane;
        int bit,nbit;

        plane = fb + pt->y * screen_info.line_length + pt->x / 16;
        bit = 1 << (14 - (pt->x & 15));
        nbit = ~bit; 
        for (i = 0; i < screen_info.bitplanes; i++, color >>= 1) {
            if (color & 1)
                *plane++ |= bit;
            else
                *plane++ &= nbit;
        }
    }
}

vdi_driver_t shifter_driver = {
    init,
    deinit,
    get_screen_info,
    set_color,
    get_color,
    resolution_has_changed,
    set_pixels
};