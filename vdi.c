#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "debug.h"
#include "fill_patterns.h"
#include "linea.h"
#include "utils.h"
#include "vdi.h"
#include "vdi_funcs.h"
#include "trap.h"

// Drivers
#include "vicky.h"
#include "shifter.h"

#define v_bas_ad 0x44e

#define DEFAULT_DRIVER &shifter_driver


#define WORKSTATIONS_SIZE 8

const workstation_features_t default_capabilities = {
    .words = {
        0,  // 0: x max
        0,  // 1: y max
        0,  // Scalability
        372,   // Pixel width in microns,
        372,   // Pixel height in microns,
        3,  // Number of character heights (0: continuous scaling)
        7,  // Number of line types
        0,  // Number of line widths (0: continuous scaling)
        6,  // Number of marker types
        0,  // Number of marker heights  (0: continuous scaling)
        1,  // Number of accessible fonts
        24, // Number of patterns
        12, // Number of hatch styles
        0,  // 13: Number of simultaneous colors
        1,  // Number of graphics primitives supported
        // 15 List of supported graphics primitives. -1 indicates end of list.
        1,  // 1 v_bar supported
        2,  // 2 v_arc supported
        3,  // 3 v_pieslice supported
        4,  // 4 v_circle supported
        5,  // 5 v_ellipse supported
        6,  // 6 v_ellarc supported
        7,  // 7 v_ellpie supported
        8,  // 8 v_rbox supported
        9,  // 9 v_rfbox supported
        10, // 10 v_justified supported
        // 24: ?
        // 25: list of attributes supported for each graphics primitive
        3,
        0,
        3,
        3,
        3,
        0,
        3,
        0,
        3,
        2
        -1, // 35: colour capability flag
        0,  // 36: text rotation capability flag
        0,  // 37: fill area capability flag
        0,  // 38: CELLARRAY capability flag
        0,  // 39: Number of color levels available. 0=32767+, 2=monochrome
        2,  // 40: Locators available for graphics cursor control (0:none, 1:keyb, 2:keyb+mouse)
        1,  // 41: Number of valuator devices for various inputs (0:nonte, 1:keyb, 2:another device)
        1,  // 42: Number of choice devices available (0:none, 1:Fn keys on keyb, 2:Fn keys+extra keypad)
        1,  // 43: Number of string devices (0:none, 1:keyboard)
        0,  // 44: Device type (0:output only, others see tos.hyp)
        5,  // 45: Minimum character width
        4,  // 46: Minimum character height
        7,  // 47: Maximum character width
        13, // 48: Maximum character height
        1,  // 49: Minimum representable line width (pixels)
        0,  // 50
        40, // 51: Max line width
        0,  // 52:
        15, // 53: Minimimum marker width
        11, // 54: Minimimum marker height
        16, // 55: Maximum marker width
        16, // 56: Maximum marker height
    }
};


// Prototypes of VDI functions ------------------------------------------------
void v_clrwk(uint16_t handle);


// Helpers
static void set_fill_pattern(workstation_settings_t *settings);
static inline void sort_corners(vdi_rectangle_t * rect);

// Utility methods -----------------------------------------------------------
void debug(const char* __restrict__ s, ...);


void vdi_install(void)
{
    trap_install();
}


void vdi_uninstall(void)
{
    trap_uninstall();
}


// Workstation management------------------------------------------------------

workstation_t workstation[WORKSTATIONS_SIZE];

void workstation_init(void) {
    int i;
    for (i=0; i<WORKSTATIONS_SIZE; i++) {
        workstation[i].handle = i;
        workstation[i].in_use = false;
    }
}


void v_opnwk(const uint16_t *input, uint16_t *handle, uint16_t *output) {
    int i;
    for (i=1; i<WORKSTATIONS_SIZE; i++) { // FIXME we start with one so we never return 0 on success (0 means failure). Wastes memory!
        workstation_t *wk = &workstation[i];

        if (wk->in_use == false) {
            wk->in_use  = true;

            // Defaults
            wk->settings.write_mode = MD_REPLACE;
            wk->settings.line_index = DEF_LINE_STYLE;
            wk->settings.fill_interior_style = DEF_FILL_STYLE;
            wk->settings.fill_pattern_hatch_style = DEF_FILL_PATTERN;
            set_fill_pattern(&wk->settings); // Needs to be called to setup internal fill stuff

            // Copy input settings. FIXME BROKEN, we'd need to copy words individually from corresponding workstation_settings_t props.
            memcpy(&(wk->settings), input, sizeof(uint16_t)*16);

            // Fill output
            *handle = i;

            // We're a physical workstation so take care of driver stuff
            wk->driver = DEFAULT_DRIVER; // TODO when we support printers etc. ;) this will have to change
            wk->physical = true;
            if (wk->physical) {
                wk->driver->init(&wk->settings);
                wk->driver->get_features(&wk->features);
            }

            screen_info_t *si = &wk->screen_info;
            wk->driver->get_screen_info(&si->max_x, &si->max_y, &si->colors, &si->line_length);

            wk->driver->get_features((workstation_features_t*)output);
            output[0] = wk->screen_info.max_x;
            output[1] = wk->screen_info.max_y;
            output[13] = wk->screen_info.colors;

            if (wk->physical)
                v_clrwk(i);

            return;
        }
    }
    *handle = 0;
    return;
}


void v_clswk(uint16_t handle) {
    workstation_t *wk = &workstation[handle];
    
    if (wk->in_use == false)
        return;

    wk->driver->deinit();
    wk->in_use = false;
}


void v_clrwk(uint16_t handle) {
    // Clear the framebuffer
    memset((void*)R32(v_bas_ad), 0, 
        workstation[handle].screen_info.line_length * (workstation[handle].screen_info.max_y + 1));
}


// Clipping -------------------------------------------------------------------

void vs_clip(uint16_t handle, uint16_t clip_flag, vdi_rectangle_t *rect) {
    workstation_t *wk = &workstation[handle];
    
    wk->settings.clip = clip_flag;
    if (wk->settings.clip) {
        sort_corners(rect);
        wk->settings.xmn_clip = MAX(0, rect->x1);
        wk->settings.ymn_clip = MAX(0, rect->y1);
        wk->settings.xmx_clip = MIN(wk->screen_info.max_x, rect->x2);
        wk->settings.ymx_clip = MIN(wk->screen_info.max_y, rect->y2);
    } else {
        wk->settings.xmn_clip = 0;
        wk->settings.ymn_clip = 0;
        wk->settings.xmx_clip = wk->screen_info.max_x;
        wk->settings.ymx_clip = wk->screen_info.max_y;
    }
}

// Colors ---------------------------------------------------------------------

void vs_color(int16_t handle, int16_t index, int16_t *rgb_in) {
    workstation[handle].driver->set_color(index, rgb_in[0], rgb_in[1], rgb_in[2]);
}


// Drawing parameters ---------------------------------------------------------

int16_t vsl_color(int16_t handle, int16_t index) {
    workstation_t *wk = &workstation[handle];
    if (index > wk->screen_info.colors)
        index = 1;    
    wk->settings.line_color = index;
    return index;
}


int16_t vsf_color(int16_t handle, int16_t index) {
    workstation_t *wk = &workstation[handle];
    if (index > wk->screen_info.colors)
        index = 1;    
    wk->settings.fill_color = index;
    return index;
}


int16_t vsf_perimeter(int16_t handle, int16_t enable) {
    workstation_t *wk = &workstation[handle];
    wk->settings.fill_perimeter = (enable != 0);
    return enable;
}


uint16_t vsf_style(uint16_t handle, uint16_t style) {
    workstation_t *wk = &workstation[handle];

    if (wk->settings.fill_interior_style == FIS_PATTERN) {
        if (style > MAX_FILL_PATTERN || style < MIN_FILL_PATTERN)
            style = DEF_FILL_PATTERN;
    } else if (style > MAX_FILL_HATCH || style < MIN_FILL_HATCH)
            style = DEF_FILL_HATCH;

    wk->settings.fill_pattern_hatch_style = style;
    set_fill_pattern(&wk->settings);
    return style;
}


uint16_t vsf_interior(uint16_t handle, uint16_t style) {
    workstation_t *wk = &workstation[handle];

    if (style < MIN_FILL_STYLE || style > MAX_FILL_STYLE)
        style = DEF_FILL_STYLE;
    wk->settings.fill_interior_style = style;
    set_fill_pattern(&wk->settings);
    return style;
}


void vqf_attributes(uint16_t handle, uint16_t *output)
{
    workstation_t *wk = &workstation[handle];

    *output++ = wk->settings.fill_interior_style;
    *output++ = wk->settings.fill_color;
    *output++ = wk->settings.fill_pattern_hatch_style;
    *output++ = wk->settings.write_mode;
    *output = wk->settings.fill_perimeter;
}


uint16_t vswr_mode(uint16_t handle, uint16_t mode) {
    workstation_t *wk = &workstation[handle];

    if (mode < MIN_WRT_MODE || mode > MAX_WRT_MODE)
        mode = DEF_WRT_MODE;
    wk->settings.write_mode = mode;
}



// Graphics Drawing Primitives ------------------------------------------------

// Rearrange corners or a rectangle so we have (lower left, upper right). Was arb_corner
static inline void sort_corners(vdi_rectangle_t * rect)
{
    /* Fix the x coordinate values, if necessary. */
    if (rect->x1 > rect->x2) {
        int16_t temp = rect->x1;
        rect->x1 = rect->x2;
        rect->x2 = temp;
    }

    /* Fix the y coordinate values, if necessary. */
    if (rect->y1 > rect->y2) {
        int16_t temp = rect->y1;
        rect->y1 = rect->y2;
        rect->y2 = temp;
    }
}


#if 0 // that shouldn't be needed as we can pass the workstation_settings_t. Unless it gets spoiled by the callee.
void Vwk2Attrib(const workstation_t *vwk, VwkAttrib *attr, const uint16_t color)
{
    /* in the same order as in Vwk, so that GCC
     * can use longs for copying words
     */
    attr->clip = vwk->clip;
    attr->multifill = vwk->multifill;
    attr->pattern_mask = vwk->pattern_mask;
    attr->pattern_ptr = vwk->pattern_ptr;
    attr->wrt_mode = vwk->wrt_mode;
    attr->color = color;
}
#endif

// Helps setup the fill pattern state variables
static void set_fill_pattern(workstation_settings_t *settings)
{
    uint16_t fi, pm;
    const uint16_t *pp = NULL;

    fi = settings->fill_index;
    pm = 0;
    switch (settings->fill_interior_style) {
    case FIS_HOLLOW:
        pp = &HOLLOW;
        break;

    case FIS_SOLID:
        pp = &SOLID;
        break;

    case FIS_PATTERN:
        if (fi < 8) {
            pm = DITHRMSK;
            pp = &DITHER[fi * (pm + 1)];
        } else {
            pm = OEMMSKPAT;
            pp = &OEMPAT[(fi - 8) * (pm + 1)];
        }
        break;
    case FIS_HATCH:
        if (fi < 6) {
            pm = HAT_0_MSK;
            pp = &HATCH0[fi * (pm + 1)];
        } else {
            pm = HAT_1_MSK;
            pp = &HATCH1[(fi - 6) * (pm + 1)];
        }
        break;
    case FIS_USER:
        pm = 0x000f;
        pp = (uint16_t*)&settings->ud_patrn[0];
        break;
    }
    settings->pattern_ptr = (uint16_t*)pp;
    settings->pattern_mask = pm;
}

// Draw filled rectangle
void vr_recfl(uint16_t handle, vdi_rectangle_t *rect)
{
    sort_corners(rect);

    // Make temporary copy to prevent the clipping code from damaging
    // the PTSIN values we might need later on for perimeter draws
    vdi_rectangle_t rect2 = *rect;

#if 0
    if (vwk->clip)
        if (!clipbox(VDI_CLIP(vwk), &rect))
            return;
#endif
    /* do the real work... */
    workstation_t *wk = &workstation[handle];
    wk->driver->draw_rectangle(&wk->settings, &rect2);
}


// Draw filled rectangle with perimeter
void v_bar(uint16_t handle, vdi_rectangle_t *rect) {
    workstation_t *wk = &workstation[handle];
    
    // vdi_line_t ordered= {
    //     40,110,55,120
    // };
    // wk->driver->draw_line(&ordered, &wk->settings, wk->settings.fill_color);
    // return;
    vr_recfl(handle, rect);

    if (wk->settings.fill_perimeter) {
        wk->settings.line_mask = 0xffff;
        vdi_point_t pts[5];
        pts[0].x = pts[4].x = pts[1].x = rect->x1;
        pts[0].y = pts[4].y = pts[3].y = rect->y1;
        pts[2].x = pts[3].x = rect->x2;
        pts[2].y = pts[1].y = rect->y2;

        polyline(wk, pts, 5, wk->settings.fill_color);
    }
}
