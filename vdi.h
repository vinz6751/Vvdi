#ifndef VDI_H
#define VDI_H

#include "fonthdr.h"
#include "helper_structs.h"
#include "features.h"

#include <stdbool.h>
#include <stdint.h>


// TOS variable pointing to framebuffer
#define v_bas_ad 0x44e

#if CONF_WITH_VIDEL
# define UDPAT_PLANES   32      /* actually 16, but each plane occupies 2 WORDs */
#elif CONF_WITH_TT_SHIFTER
# define UDPAT_PLANES   8
#else
# define UDPAT_PLANES   4
#endif

/* Fill interior style (as set by vsf_interior) */
#define FIS_HOLLOW  0
#define FIS_SOLID   1
#define FIS_PATTERN 2
#define FIS_HATCH   3
#define FIS_USERDEF 4

extern const workstation_features_t default_capabilities;

typedef struct {
    void     *driver_data;          // The driver does what it wants with that
    uint16_t line_type;
    uint16_t line_mask;          // For dashed/dotted lines. Is derived from line_index and ud_ls
    uint16_t line_marker_type;
    uint16_t line_marker_colour;
    uint16_t line_font_id;
    uint16_t line_text_colour;
    uint16_t fill_interior_style; // FIS_xxxx values (hollow, solid, pattern, hatch, user)
    uint16_t fill_pattern_hatch_style;
    uint16_t fill_color;
    uint16_t coordinates_type; // 0 = Normalized, 2 = Raster (normal use)
    uint16_t page_format;

    int16_t chup;                  /* Character Up vector */
    int16_t clip;                  /* Clipping Flag */
    fonthead_t *cur_font;      /* Pointer to current font */
    uint16_t dda_inc;              /* Fraction to be added to the DDA */
    int16_t multifill;             /* Multi-plane fill flag */
    uint16_t pattern_mask;         /* (pattern_mask) Current pattern mask */
    uint16_t *pattern_ptr;         /* (pattern_ptr) Current pattern pointer */
    int16_t pts_mode;              /* TRUE if height set in points mode */
    int16_t *scrtchp;              /* Pointer to text scratch buffer */
    int16_t scrpt2;                /* Offset to large text buffer */
    int16_t text_style;            /* Current text style */
    int16_t t_sclsts;              /* TRUE if scaling up */
    int16_t fill_index;            /* Current fill index */
    int16_t fill_perimeter;        /* (fill_per) draw perimeter/outline on filled areas */
//    int16_t fill_style;          /* Current fill style */
    int16_t h_align;               /* Current text horizontal alignment */
    int16_t line_beg;              /* Beginning line endstyle */
    int16_t line_color;            /* Current line color (PEL value) */
    int16_t line_end;              /* Ending line endstyle */
    int16_t line_index;            /* Current line style   */
    int16_t line_width;            /* Current line width   */
    const fonthead_t *loaded_fonts;  /* Pointer to first loaded font   */
    struct {
        int16_t color;            /* Current marker color (PEL value) */
        int16_t height;           /* Current marker height            */
        int16_t index;            /* Current marker style             */
        int16_t scale;            /* Current scale factor for marker data */
    } mark;
    int16_t num_fonts;             /* Total number of faces available    */
    int16_t scaled;                /* TRUE if font scaled in any way     */
    fonthead_t scratch_head;         /* Holder for the doubled font data */
    int16_t text_color;            /* Current text color (PEL value)      */
    int16_t ud_ls;                 /* User defined linestyle              */
    int16_t ud_patrn[UDPAT_PLANES*16]; /* User-defined fill pattern       */
    int16_t v_align;               /* Current text vertical alignment     */
    int16_t write_mode;            /* Current writing mode                */
    int16_t xfm_mode;              /* Transformation mode requested (NDC) */
    vdi_clipping_rect_t clip_rect; /* Clipping rectangle */

} workstation_settings_t;

// Internal structure used to hold info about the framebuffer
typedef struct {
    uint16_t max_x;
    uint16_t max_y;
    uint32_t colors;
    uint16_t line_length; // Length of a line in the frame buffer, in bytes
    uint8_t  bitplanes;   // Number of bitplanes (will be assumed interleaved Atari ST-style if > 1)
    struct {
        uint16_t width; // Dimension of a pixel in micro-meters
        uint16_t height;
    } pixel;
} screen_info_t;

// For device drivers to implement
typedef struct {
    void (*init)(workstation_settings_t *settings);
    void (*deinit)(void);
    void (*get_features)(workstation_features_t *out_features);
    void (*get_screen_info)(uint16_t *resx, uint16_t *resy, uint32_t *ncolors, uint16_t *line_length);
    void (*set_color)(uint16_t n, uint16_t red, uint16_t green, uint16_t blue);
    void (*get_color)(uint16_t n, uint16_t *red, uint16_t *green, uint16_t *blue);
    // Let the VDI know that the video resolution has changed. VDI should examine hardware registers
    // and update its internal variables.
    void (*resolution_has_changed)(void);
    // Get the palette index and hardware value of a pixel
    void (*get_pixels)(const vdi_point_t *pts, uint16_t n, int16_t *color, int16_t *pixel);
    // Sets the pixels to the specified color number
    void (*set_pixels)(const vdi_point_t *pts, uint16_t n, uint16_t color);
    void (*draw_line)(const vdi_line_t *line, workstation_settings_t *settings, uint16_t color);
    void (*draw_rectangle)(const workstation_settings_t *settings, const vdi_rectangle_t *rect);
} vdi_driver_t;

// Internal representation of a workstation (virtual or not)
typedef struct _workstation_t {
    bool     in_use;
    bool     physical;
    struct _workstation_t *phys_wk; // If this wk is virtual, this is a link to the physical one
    vdi_driver_t *driver;
    workstation_features_t features; // Filled by the driver, correspond to work_out
    screen_info_t screen_info;
    uint16_t handle;
    // Everything below is laid out the same as in the input of opening a workstation so we can copy memory during opnwk
    workstation_settings_t settings; // Current settings (as set by user) and state of the workstation
    char*    filename;
    uint16_t reserved14;
    uint16_t reserved15;
} workstation_t;

typedef struct mfdb
{
   void *fd_addr;                /* Pointer to the start of the memory block, e.g. the screen memory base address  */
   int16_t  fd_w;                /* Width in pixels             */
   int16_t  fd_h;                /* Height in pixels            */
   int16_t  fd_wdwidth;          /* Width of a line in words    */
   int16_t  fd_stand;            /* 0 = Device-specific format  */
                                 /* 1 = Standard format         */
   int16_t  fd_nplanes;          /* Number of planes            */
   int16_t  fd_r1, fd_r2, fd_r3; /* Reserved, must be 0         */
} MFDB;

extern workstation_t workstation[];

// Helpers, TODO should be moved ? 
void sort_corners(vdi_rectangle_t * rect);

#endif
