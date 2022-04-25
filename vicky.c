#include <stdint.h>
#include "utils.h"

// VICKY2 driver --------------------------------------------------------------
#define VICKY2              0xB40000L
#define VICKY2_CTRL         (VICKY2)
#define VICKY2_BORDER_CTRL  (VICKY2+0x04)
#define VICKY2_BACKGROUND   (VICKY2+0x0c)
#define VICKY2_CURSOR_CTRL  (VICKY2+0x10)
#define VICKY2_BMP1_CTRL    (VICKY2+0x108)
#define VICKY2_BMP1_ADDR    (VICKY2+0x10c)
#define VICKY_LUT_BASE      (VICKY2+0x2000)

uint32_t old_vicky_ctrl;
uint32_t old_vicky_border_ctrl;
uint32_t old_vicky_background;
uint32_t old_vicky_cursor_ctrl;
uint32_t old_vicky_bmp1_ctrl;
uint32_t old_vicky_bmp1_addr;


void vicky_init_fb(void) {
    old_vicky_ctrl = R32(VICKY2_CTRL);
    R32(VICKY2_CTRL) &= ~0x00000301L;  // Reset resolution, disable text mode (just in case)
    R32(VICKY2_CTRL) |= 0x0100/*800*600*/ | 4|8; // Bitmap + gfx engine
    old_vicky_border_ctrl = R32(VICKY2_BORDER_CTRL);
    R32(VICKY2_BORDER_CTRL) = 0; // No border

    old_vicky_background = R32(VICKY2_BACKGROUND);
    R32(VICKY2_BACKGROUND) = 0xffffffff; // White
    
    old_vicky_cursor_ctrl = R32(VICKY2_CURSOR_CTRL);

    // Enable bitmap layer 1 (background), LUT0
    old_vicky_bmp1_ctrl = R32(VICKY2_BMP1_CTRL);
    R32(VICKY2_BMP1_CTRL) = 1;
    old_vicky_bmp1_addr = R32(VICKY2_BMP1_ADDR);
    R32(VICKY2_BMP1_ADDR) = 0; // Start of VRAM
}

void vicky_deinit_fb(void) {
    R32(VICKY2_BORDER_CTRL) = old_vicky_border_ctrl;
    R32(VICKY2_CTRL) = old_vicky_ctrl;
    R32(VICKY2_BACKGROUND) = old_vicky_background;
    R32(VICKY2_CURSOR_CTRL) = old_vicky_cursor_ctrl;
    R32(VICKY2_BMP1_CTRL) = old_vicky_bmp1_ctrl;
    R32(VICKY2_BMP1_ADDR) = old_vicky_bmp1_addr;
}

void vicky_get_screen_info(uint16_t *resx, uint16_t *resy, uint32_t *ncolors, uint16_t *line_length) {
    *resx = 640;
    *resy = 480;
    *ncolors = 256;
    *line_length = *resx;
}

#define LUT_ADDRESS(lut, n) (uint8_t*)(VICKY_LUT_BASE + lut*0x400 + n*4)

// Set color on LUT 0, colors are 8 bits each
void vicky_set_color(uint16_t n, uint16_t red, uint16_t green, uint16_t blue) {
    const int lut = 0;
    volatile uint8_t * c = LUT_ADDRESS(lut,n);
    c[0] = blue;
    c[1] = green;
    c[2] = red;
    c[3] = 0xff;
}

void vicky_get_color(uint16_t n, uint16_t *red, uint16_t *green, uint16_t *blue) {
    const int lut = 0;
    volatile uint8_t * c = LUT_ADDRESS(lut,n);
    *blue = c[0];
    *green = c[1];
    *red = c[2];
}