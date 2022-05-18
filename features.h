// Describes features and settings of a workstation
#ifndef FEATURES_H
#define FEATURES_H

#include <stdint.h>

#define MIN_FILL_STYLE  0       /* for vsf_interior() */
#define FIS_HOLLOW      0
#define FIS_SOLID       1
#define FIS_PATTERN     2
#define FIS_HATCH       3
#define FIS_USER        4
#define MAX_FILL_STYLE  4
#define DEF_FILL_STYLE  FIS_HOLLOW

#define MIN_FILL_HATCH  1       /* for vsf_style() when fill style is hatch */
#define MAX_FILL_HATCH  12
#define DEF_FILL_HATCH  1

#define MIN_FILL_PATTERN 1      /* for vsf_style() when fill style is pattern */
#define MAX_FILL_PATTERN 24
#define DEF_FILL_PATTERN 1

#define MIN_WRT_MODE    1       /* for vswr_mode() */
#define MD_REPLACE      1
#define MD_TRANS        2
#define MD_XOR          3
#define MD_ERASE        4
#define MAX_WRT_MODE    4
#define DEF_WRT_MODE    MD_REPLACE


// This is really the "work_out" array filled when opening a workstation
typedef union {
    uint16_t words[57];
    struct {
        uint16_t xmax;
        uint16_t ymax;
        uint16_t scalability;
        uint16_t pixel_width;   // in microns
        uint16_t pixel_height;
        uint16_t n_pixel_heights; // 0: continuous scaling
        uint16_t n_line_types;
        uint16_t n_line_widths; // 0: continuous scaling
        uint16_t n_marker_types;
        uint16_t n_marker_heights; // 0: continuous scaling
        uint16_t n_fonts;
        uint16_t n_patterns;
        uint16_t n_hatch_types;
        uint16_t n_simultaneous_colors; // simultaneous colors
        uint16_t n_gdp; // number of graphics primitives supported
        uint16_t has_v_bar;
        uint16_t has_v_arc;
        uint16_t has_pieslive;
        uint16_t has_v_circle;
        uint16_t has_v_ellipse;
        uint16_t has_v_ellarc;
        uint16_t has_v_ellpie;
        uint16_t has_v_rbox;
        uint16_t has_v_rfbox;
        uint16_t has_v_justified;
        uint16_t word24;
        uint16_t attr_v_bar;
        uint16_t attr_v_arc;
        uint16_t attr_pieslive;
        uint16_t attr_v_circle;
        uint16_t attr_v_ellipse;
        uint16_t attr_v_ellarc;
        uint16_t attr_v_ellpie;
        uint16_t attr_v_rbox;
        uint16_t attr_v_rfbox;
        uint16_t attr_v_justified;
        uint16_t colors_capable;
        uint16_t rotations_capable;
        uint16_t fill_capable;
        uint16_t cellarray_capable;
        uint16_t n_colors;  // 0=32767+, 2=monochrome
        uint16_t locators;  // Locators available for graphics cursor control (0:none, 1:keyb, 2:keyb+mouse)
        uint16_t valuators; // Number of valuator devices for various inputs (0:nonte, 1:keyb, 2:another device)
        uint16_t choices;   // Number of choice devices available (0:none, 1:Fn keys on keyb, 2:Fn keys+extra keypad)
        uint16_t strings;   // Number of string devices (0:none, 1:keyboard)
        uint16_t device_type; // Device type (0:output only, others see tos.hyp)
        uint16_t min_char_width;
        uint16_t min_char_height;
        uint16_t max_char_width;
        uint16_t max_char_height;
        uint16_t min_line_width; // in pixels
        uint16_t word50;
        uint16_t max_line_width;
        uint16_t word52;
        uint16_t min_marker_width;
        uint16_t min_marker_height;
        uint16_t max_marker_width;
        uint16_t max_marker_height;
    };
} workstation_features_t;

#endif