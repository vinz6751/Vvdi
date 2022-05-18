#include <stdint.h>
#include "utils.h"
#include "vdi.h"

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

static void init(workstation_settings_t *settings) {
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

static void deinit(void) {
    R32(VICKY2_BORDER_CTRL) = old_vicky_border_ctrl;
    R32(VICKY2_CTRL) = old_vicky_ctrl;
    R32(VICKY2_BACKGROUND) = old_vicky_background;
    R32(VICKY2_CURSOR_CTRL) = old_vicky_cursor_ctrl;
    R32(VICKY2_BMP1_CTRL) = old_vicky_bmp1_ctrl;
    R32(VICKY2_BMP1_ADDR) = old_vicky_bmp1_addr;
}

static void get_features(workstation_features_t *features) {
    for (int i=0; i<57 ; i++)
        features->words[i] = default_capabilities.words[i];
}

static void get_screen_info(uint16_t *resx, uint16_t *resy, uint32_t *ncolors, uint16_t *line_length) {
    *resx = 640;
    *resy = 480;
    *ncolors = 256;
    *line_length = *resx;
}

#define LUT_ADDRESS(lut, n) (uint8_t*)(VICKY_LUT_BASE + lut*0x400 + n*4)

// Set color on LUT 0, colors are 8 bits each
static void set_color(uint16_t n, uint16_t red, uint16_t green, uint16_t blue) {
    const int lut = 0;
    volatile uint8_t * c = LUT_ADDRESS(lut,n);
    c[0] = blue;
    c[1] = green;
    c[2] = red;
    c[3] = 0xff;
}

static void get_color(uint16_t n, uint16_t *red, uint16_t *green, uint16_t *blue) {
    const int lut = 0;
    volatile uint8_t * c = LUT_ADDRESS(lut,n);
    *blue = c[0];
    *green = c[1];
    *red = c[2];
}

static screen_info_t screen_info;

static void resolution_has_changed(void) {
    uint32_t res = (R32(0x00b40000) >> 8) & 3;
    screen_info.colors = 256;
    screen_info.bitplanes = 1;
    
    switch (res) {
        case 0:
            screen_info.max_x = 640-1;
            screen_info.max_y = 480-1;
            break;
        case 1:
            screen_info.max_x = 800-1;
            screen_info.max_y = 600-1;
            break;
        case 2: // Not supported on the U
            screen_info.max_x = 1024-1;
            screen_info.max_y = 768-1;
            break;
        case 3:
            screen_info.max_x = 640-1;
            screen_info.max_y = 400-1;
            break;
    }
    screen_info.line_length = screen_info.max_x + 1;
}

static void set_pixels(const vdi_point_t *pts, uint16_t n, uint16_t color) {
    uint8_t *fb = (uint8_t*)R32(v_bas_ad);
    vdi_point_t *pt;
    
    while (n++) {
        pt = (vdi_point_t *)pts++;
        fb + pt->y * screen_info.line_length + pt->x;
    }
}

static void draw_line(const vdi_line_t *line, workstation_settings_t *settings, uint16_t color) {
    // TODO    
}

vdi_driver_t vicky2_driver = {
    init,
    deinit,
    get_features,
    get_screen_info,
    set_color,
    get_color,
    resolution_has_changed,
    set_pixels,
    draw_line,
    0L
};