#ifndef VDI_H
#define VDI_H

#include <stdbool.h>
#include <stdint.h>


// TOS variable pointing to framebuffer
#define v_bas_ad 0x44e

// Internal structure used to hold info about the framebuffer
typedef struct {
    uint16_t max_x;
    uint16_t max_y;
    uint32_t colors;
    uint16_t line_length; // Length of a line in the frame buffer, in bytes
    uint8_t  bitplanes; // Number of bitplanes (will be assumed interleaved Atari ST-style if > 1)
} screen_info_t;


// Used to represent a point internally
typedef struct {
    uint16_t x;
    uint16_t y;
} vdi_point_t;


// Clipping settings for a workstation
typedef struct {
    vdi_point_t p1;
    vdi_point_t p2;
    bool     enabled;
} clip_settings_t;


typedef union {
    struct {
        uint16_t opcode;
        uint16_t ptsin_count;
        uint16_t ptsout_count;
        uint16_t intin_count;
        uint16_t intout_count;
        uint16_t subopcode;
        uint16_t wkid;
    };
    uint16_t word[12];
} vdi_pb_contrl_t;

// VDI parameter block for callers.
typedef struct
{
    vdi_pb_contrl_t *contrl;
    uint16_t *intin;
    union {
        uint16_t *words;
        vdi_point_t *pts;
    } ptsin;
    uint16_t *intout;
    union {
        uint16_t *words;
        vdi_point_t *pts;
    } ptsout;
} vdi_pb_t;

// For device drivers to implement
typedef struct {
    void (*init)(void);
    void (*deinit)(void);
    void (*get_screen_info)(uint16_t *resx, uint16_t *resy, uint32_t *ncolors, uint16_t *line_length);
    void (*set_color)(uint16_t n, uint16_t red, uint16_t green, uint16_t blue);
    void (*get_color)(uint16_t n, uint16_t *red, uint16_t *green, uint16_t *blue);
    // Let the VDI know that the video resolution has changed. VDI should examine hardware registers
    // and update its internal variables.
    void (*resolution_has_changed)(void);
    // Sets the pixels to the specified color number
    void (*set_pixels)(const vdi_point_t *pts, uint16_t n, uint16_t color);
} vdi_driver_t;

#endif
