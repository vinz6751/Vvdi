#include <stdint.h>
#include <stdbool.h>

#include "assignsys.h"
#include "attribute.h"
#include "debug.h"
#include "fill_patterns.h"
#include "font.h"
#include "linea.h"
#include "memory.h"
#include "utils.h"
#include "vdi.h"
#include "vdi_funcs.h"
#include "trap.h"



#define v_bas_ad 0x44e

// Preferences

#define MEM_BLOCKS        2 /* Default number of memory blocks to allocate for internal use */
#define MEM_BLOCK_SIZE    10 /* Default size of those blocks, in kbyte */


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


// Helpers

void sort_corners(vdi_rectangle_t * rect);

// Utility methods -----------------------------------------------------------


bool vdi_install(void)
{
    // Do that first as it can fail
    mem_initialize(MEM_BLOCKS, MEM_BLOCK_SIZE*1024L);
    
    assignsys_init();
    font_init();
    trap_install();
    _debug("installed");
    return true;
}


void vdi_uninstall(void)
{
    trap_uninstall();
    font_deinit();
    assignsys_deinit();
    _debug("uninstalled");
}


// Clipping -------------------------------------------------------------------

void vs_clip(uint16_t handle, uint16_t clip_flag, vdi_rectangle_t *rect)
{
    workstation_t *wk = &workstation[handle];
    
    wk->settings.clip = clip_flag;
    if (wk->settings.clip)
    {
        sort_corners(rect);
        wk->settings.clip_rect.xmin = MAX(0, rect->x1);
        wk->settings.clip_rect.ymin = MAX(0, rect->y1);
        wk->settings.clip_rect.xmax = MIN(wk->screen_info.max_x, rect->x2);
        wk->settings.clip_rect.ymax = MIN(wk->screen_info.max_y, rect->y2);
    }
    else {
        wk->settings.clip_rect.xmin = 0;
        wk->settings.clip_rect.ymin = 0;
        wk->settings.clip_rect.xmax = wk->screen_info.max_x;
        wk->settings.clip_rect.ymax = wk->screen_info.max_y;
    }
}




// Graphics Drawing Primitives ------------------------------------------------

// Rearrange corners or a rectangle so we have (lower left, upper right). Was arb_corner
void sort_corners(vdi_rectangle_t * rect)
{
    /* Fix the x coordinate values, if necessary. */
    if (rect->x1 > rect->x2)
    {
        int16_t temp = rect->x1;
        rect->x1 = rect->x2;
        rect->x2 = temp;
    }

    /* Fix the y coordinate values, if necessary. */
    if (rect->y1 > rect->y2)
    {
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
void v_bar(uint16_t handle, vdi_rectangle_t *rect)
{
    workstation_t *wk = &workstation[handle];

    vr_recfl(handle, rect);

    if (wk->settings.fill_perimeter)
    {
        wk->settings.line_mask = 0xffff;
        vdi_point_t pts[5];
        pts[0].x = pts[4].x = pts[1].x = rect->x1;
        pts[0].y = pts[4].y = pts[3].y = rect->y1;
        pts[2].x = pts[3].x = rect->x2;
        pts[2].y = pts[1].y = rect->y2;

        polyline(wk, pts, 5, wk->settings.fill_color);
    }
}
