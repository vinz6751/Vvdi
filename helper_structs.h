// Helper structures, which make it easier than using array of ints.

#ifndef HELPER_STRUCTS_H
#define HELPER_STRUCTS_H

#include <stdint.h>

// Used to represent a point internally
typedef struct {
    uint16_t x;
    uint16_t y;
} vdi_point_t;

typedef struct {
    int16_t x1,y1;
    int16_t x2,y2;
} vdi_line_t;

// Used to represent a rectangle internally
typedef struct {
    uint16_t x1,y1;
    uint16_t x2,y2;
} vdi_rectangle_t;

#endif