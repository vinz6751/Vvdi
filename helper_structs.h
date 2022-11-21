// Helper structures, which make it easier than using array of ints.

#ifndef HELPER_STRUCTS_H
#define HELPER_STRUCTS_H

#include <stdint.h>

// Used to represent a point internally
typedef struct {
    uint16_t x;
    uint16_t y;
} vdi_point_t;

// Represents a line
typedef struct {
    int16_t x1,y1;
    int16_t x2,y2;
} vdi_line_t;

// Used to represent a rectangle internally
typedef struct {
    uint16_t x1,y1;
    uint16_t x2,y2;
} vdi_rectangle_t;

// Represents a clipping rectangle. Do not change this as it represents a part of the vdi_workstation_t struct.
// The equivalent in EmuTOS is VwkClip.
typedef struct {
    int16_t xmin;              /* Low x point of clipping rectangle    */
    int16_t xmax;              /* High x point of clipping rectangle   */
    int16_t ymin;              /* Low y point of clipping rectangle    */
    int16_t ymax;              /* High y point of clipping rectangle   */
} vdi_clipping_rect_t;

// TODO this is copied from fvdi but not used yet. I don't understand why colors need to be 
// represented by this rather than just a color number.
typedef struct Fgbg_ {
    int16_t background;
    int16_t foreground;
} Fgbg;

#endif