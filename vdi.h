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

extern const workstation_features_t default_capabilities;

typedef struct  {
    void     *driver_data;          // The driver does what it wants with that
    uint16_t line_type;
    uint16_t line_mask;  // For dashed/dotted lines. Is derived from line_index and ud_ls
    uint16_t line_marker_type;
    uint16_t line_marker_colour;
    uint16_t line_font_id;
    uint16_t line_text_colour;
    uint16_t fill_interior_style;
    uint16_t fill_pattern_hatch_style;
    uint16_t fill_color;
    uint16_t coordinates_type; // 0 = Normalized, 2 = Raster (normal use)
    uint16_t page_format;

    int16_t chup;                  /* Character Up vector */
    int16_t clip;                  /* Clipping Flag */
    const Fonthead *cur_font;   /* Pointer to current font */
    uint16_t dda_inc;              /* Fraction to be added to the DDA */
    int16_t multifill;             /* Multi-plane fill flag */
    uint16_t pattern_mask;         /* (pattern_mask) Current pattern mask */
    uint16_t *pattern_ptr;              /* (pattern_ptr) Current pattern pointer */
    int16_t pts_mode;              /* TRUE if height set in points mode */
    int16_t *scrtchp;              /* Pointer to text scratch buffer */
    int16_t scrpt2;                /* Offset to large text buffer */
    int16_t style;                 /* Current text style */
    int16_t t_sclsts;              /* TRUE if scaling up */
    int16_t fill_index;            /* Current fill index */
    int16_t fill_perimeter;              /* (fill_per) draw perimeter/outline on filled areas */
//    int16_t fill_style;            /* Current fill style */
    int16_t h_align;               /* Current text horizontal alignment */
    int16_t line_beg;              /* Beginning line endstyle */
    int16_t line_color;            /* Current line color (PEL value) */
    int16_t line_end;              /* Ending line endstyle */
    int16_t line_index;            /* Current line style */
    int16_t line_width;            /* Current line width */
    const Fonthead *loaded_fonts;  /* Pointer to first loaded font */
    int16_t mark_color;            /* Current marker color (PEL value)     */
    int16_t mark_height;           /* Current marker height        */
    int16_t mark_index;            /* Current marker style         */
    int16_t mark_scale;            /* Current scale factor for marker data */
    int16_t num_fonts;             /* Total number of faces available  */
    int16_t scaled;                /* TRUE if font scaled in any way   */
    Fonthead scratch_head;         /* Holder for the doubled font data */
    int16_t text_color;            /* Current text color (PEL value)   */
    int16_t ud_ls;                 /* User defined linestyle       */
    int16_t ud_patrn[UDPAT_PLANES*16]; /* User defined pattern             */
    int16_t v_align;               /* Current text vertical alignment  */
    int16_t write_mode;            /* Current writing mode         */
    int16_t xfm_mode;              /* Transformation mode requested (NDC) */
    int16_t xmn_clip;              /* Low x point of clipping rectangle    */
    int16_t xmx_clip;              /* High x point of clipping rectangle   */
    int16_t ymn_clip;              /* Low y point of clipping rectangle    */
    int16_t ymx_clip;              /* High y point of clipping rectangle   */
} workstation_settings_t;

// Internal structure used to hold info about the framebuffer
typedef struct {
    uint16_t max_x;
    uint16_t max_y;
    uint32_t colors;
    uint16_t line_length; // Length of a line in the frame buffer, in bytes
    uint8_t  bitplanes; // Number of bitplanes (will be assumed interleaved Atari ST-style if > 1)
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
    // Sets the pixels to the specified color number
    void (*set_pixels)(const vdi_point_t *pts, uint16_t n, uint16_t color);
    void (*draw_line)(const vdi_line_t *line, workstation_settings_t *settings, uint16_t color);
    void (*draw_rectangle)(const workstation_settings_t *settings, const vdi_rectangle_t *rect);
} vdi_driver_t;

// Internal representation of a workstation (virtual or not)
typedef struct {
    bool     in_use;
    bool     physical;
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

extern workstation_t workstation[];

#endif
