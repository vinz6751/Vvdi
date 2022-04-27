#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <osbind.h>

#include "linea.h"
#include "utils.h"
#include "vdi.h"
#include "vicky.h"


#define v_bas_ad 0x44e


typedef struct {
    uint16_t max_x;
    uint16_t max_y;
    uint32_t colors;
    uint16_t line_length; // Length of a line in the frame buffer, in bytes
} screen_info_t;


#define PTSIN_SIZE 256
#define PTSOUT_SIZE 256
#define INTIN_SIZE 256
#define INTOUT_SIZE 256
#define WORKSTATIONS_SIZE 8

typedef struct {
    union {
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
    } contrl;
    uint16_t intin[INTIN_SIZE];
    union {
        uint16_t words[PTSIN_SIZE];
        vdi_point_t pts[(PTSIN_SIZE*sizeof(uint16_t))/sizeof(vdi_point_t)];
    } ptsin;
    uint16_t intout[INTOUT_SIZE];
    union {
        uint16_t words[PTSOUT_SIZE];
        vdi_point_t pts[(PTSOUT_SIZE*sizeof(uint16_t))/sizeof(vdi_point_t)];
    } ptsout;
} vdi_parameter_block_t;


typedef struct {
    bool     in_use;
    clip_settings_t clip;
    screen_info_t screen_info;
    bool          draw_perimeter;

// In, layed out the same as in the input of opening a workstation so we can copy memory during opnwk
    uint16_t handle;
    uint16_t line_type;
    uint16_t line_colour;
    uint16_t line_marker_type;
    uint16_t line_marker_colour;
    uint16_t line_font_id;
    uint16_t line_text_colour;
    uint16_t line_fill_interior_style;
    uint16_t line_fill_style;
    uint16_t line_fill_colour;
    uint16_t coordinates_type; // 0 = Normalized, 2 = Raster (normal use)
    uint16_t page_format;
    char*    filename;
    uint16_t reserved14;
    uint16_t reserved15;
} workstation_t;

const uint16_t work_out[] = {
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
};




// Prototypes of VDI functions ------------------------------------------------
// The vdi_xxx functions receive a VDI parameter block
static void vdi_v_opnwk(vdi_parameter_block_t *pb);
static void vdi_v_clswk(vdi_parameter_block_t *pb);
static void vdi_v_clrwk(vdi_parameter_block_t *pb);

static void v_clrwk(uint16_t handle);
static void vdi_vs_clip(vdi_parameter_block_t *pb);
static void vdi_vs_color(vdi_parameter_block_t *pb);
static void vdi_vsl_color(vdi_parameter_block_t *pb);

// Vector installation --------------------------------------------------------
extern void vdi_trap_handler(void);

#define TRAP2_VECNR 34
extern void (*old_trap_handler)();

void vdi_install(void)
{
    old_trap_handler = Setexc(TRAP2_VECNR, -1L);
    Setexc(TRAP2_VECNR,vdi_trap_handler);
}


void vdi_uninstall(void)
{
    old_trap_handler = Setexc(TRAP2_VECNR, -1L);
    Setexc(TRAP2_VECNR,vdi_trap_handler);  
}

// Trap handler function dispatcher -------------------------------------------
void vdi_dispatcher(vdi_parameter_block_t *pb) {    
    void (*vdi_calls[])(vdi_parameter_block_t *pb) = {
        0L,
        vdi_v_opnwk, // 1
        vdi_v_clswk, // 2
        vdi_v_clrwk, // 3
    };

    switch (pb->contrl.opcode) {
        case 14: vdi_vs_color(pb); break;
        case 17: vdi_vsl_color(pb); break;
        case 129: vdi_vs_clip(pb); break;
        default:
            (*vdi_calls[pb->contrl.opcode])(pb);
            return;
    }    
}



// Workstation management------------------------------------------------------

workstation_t workstation[WORKSTATIONS_SIZE];

void workstation_init(void) {
    for (int i=0; i<WORKSTATIONS_SIZE; i++) {
        workstation[i].handle = i;
        workstation[i].in_use = false;
        workstation[i].draw_perimeter = true;
    }
}


static void v_opnwk(const uint16_t *input, uint16_t *handle, uint16_t *output) {
    for (int i=0; i<WORKSTATIONS_SIZE; i++) {
        if (workstation[i].in_use == false) {
            workstation[i].in_use  = true;
            // Copy input settings
            memcpy(&(workstation[i].line_type), input, sizeof(uint16_t)*16);

            // Fill output
            memcpy(output, work_out, sizeof(work_out));            
            *handle = i;

            vicky_init_fb();
            screen_info_t *si = &workstation[i].screen_info;
            vicky_get_screen_info(&si->max_x, &si->max_y, &si->colors, &si->line_length);
            
            output[0] = workstation[i].screen_info.max_x;
            output[1] = workstation[i].screen_info.max_y;
            output[13] = workstation[i].screen_info.colors;

            v_clrwk(i);

            return;
        }
    }
    return;
}
static void vdi_v_opnwk(vdi_parameter_block_t *pb) {
    v_opnwk(pb->intin, &pb->contrl.wkid, pb->intout);
}


static void v_clswk(uint16_t handle) {
    if (workstation[handle].in_use == false)
        return;

    vicky_deinit_fb();
}
static void vdi_v_clswk(vdi_parameter_block_t *pb) {
    v_clswk(pb->contrl.wkid);
}


static void v_clrwk(uint16_t handle) {
    // Set everything transparent. TODO the below is VICKY specific,
    // it doesn't belong here.
    memset((void*)R32(v_bas_ad), 0, 
        (workstation[handle].screen_info.max_x + 1) * (workstation[handle].screen_info.max_y + 1));
}
static void vdi_v_clrwk(vdi_parameter_block_t *pb) {
    v_clrwk(pb->contrl.wkid);
}


// Clipping -------------------------------------------------------------------

static void clip_init(workstation_t *wk) {
    wk->clip.enabled = 0;
    wk->clip.p1.x = wk->clip.p1.y = 0;
    wk->clip.p2.x = wk->screen_info.max_x;
    wk->clip.p2.y = wk->screen_info.max_y;
    wk->clip.enabled = true;
}

static void vs_clip(uint16_t handle, uint16_t clip_flag, const vdi_point_t *pts) {
    workstation_t *wk = &workstation[handle];
    
    if (wk->clip.enabled = clip_flag != 0) {
        wk->clip.p1 = pts[0];
        wk->clip.p2 = pts[1];
    }
}
static void vdi_vs_clip(vdi_parameter_block_t *pb) {
    vs_clip(pb->contrl.wkid, pb->intin[0], pb->ptsin.pts);
}


static inline bool clip(workstation_t *wk, uint16_t x, uint16_t y) {
    return (!wk->clip.enabled) || (
        (x >= wk->clip.p1.x) &&
        (x <= wk->clip.p2.x) &&
        (y >= wk->clip.p1.y) &&
        (y <= wk->clip.p2.y));
}

// Colors ---------------------------------------------------------------------

static void vs_color(int16_t handle, int16_t index, int16_t *rgb_in) {    
    vicky_set_color(index, rgb_in[0] >> 2, rgb_in[1] >> 2, rgb_in[2] >> 2);
}
static void vdi_vs_color(vdi_parameter_block_t *pb) {
    vs_color(pb->contrl.wkid, pb->intin[0], &pb->intin[1]);
}


static int16_t vsl_color(int16_t handle, int16_t index) {    
    workstation_t *wk = &workstation[handle];
    if (index > wk->screen_info.colors)
        index = 1;    
    wk->line_colour = index;
    return index;
}
static void vdi_vsl_color(vdi_parameter_block_t *pb) {
    vsl_color(pb->contrl.wkid, pb->intin[0]);
}


// Graphics Drawing Primitives ------------------------------------------------

static void v_bar(uint16_t handle, vdi_point_t *pts) {
    workstation_t *wk = &workstation[handle];
    int from_x, to_x;
    int from_y, to_y;
    int w,h;

    // Ensure from <= to
    if (pts[0].x > pts[1].x) {
        from_x = pts[1].x;
        to_x = pts[0].x;
    }
    if (pts[0].y > pts[1].y) {
        from_y = pts[1].y;
        to_y = pts[0].y;
    }
    
    // Clip rectangle
    if (wk->clip.enabled) {
        from_x = MAX(from_x, wk->clip.p1.x);
        from_x = MAX(from_y, wk->clip.p1.y);
        to_x = MIN(to_x, wk->clip.p2.x);
        to_y = MIN(to_y, wk->clip.p2.y);
    }

    h = to_y - from_y;
    if (h < 0)
        return;
    
    w = to_x - from_x;
    if (w < 0)
        return;
    
    uint8_t *fb = *((uint8_t**)v_bas_ad);
    uint8_t *top_left = &fb[wk->screen_info.line_length * from_y + from_x];


    uint16_t i;

    // Draw perimeter
    if (wk->draw_perimeter) {
        const int total_border_size = 2;
        uint8_t *top_border = top_left;
        uint8_t *btm_border = top_left + wk->screen_info.line_length * h;    
        // Top/bottom borders
        i = w;
        do {
            *top_border++ = (uint8_t)wk->line_fill_colour;
            *btm_border++ = (uint8_t)wk->line_fill_colour;
        } while (i--);
        from_y++;
        to_y--;

        // Left/right borders
        i = h - total_border_size;
        uint8_t *left_border = top_left + wk->screen_info.line_length;
        uint8_t *right_border = left_border + w;
        do {
            *left_border++ = (uint8_t)wk->line_fill_colour;
            *right_border++ = (uint8_t)wk->line_fill_colour;
        } while (i--);
        from_x++;
        to_x--;
    }

}

// Main -----------------------------------------------------------------------
static void tests(void);
uint16_t call_vdi(vdi_parameter_block_t *pb);

int main(void)
{
    // So we can get access to fonts
    linea_init();

    // Install trap handler
    vdi_install();

    tests();
    
    vdi_uninstall();

    return 0;
}


static void tests(void) {
    vdi_parameter_block_t pb;
    int i;
    uint16_t work_in[11],work_out[57];
    uint16_t handle;

    // Open workstation
    pb.contrl.opcode = 1;
    pb.contrl.ptsin_count = 0;
    pb.contrl.intin_count = 11;
    pb.contrl.intout_count = 57;
    for(i = 0;i < 11;i++)
        pb.intin[i] = work_in[i];
    call_vdi(&pb);
    handle = pb.contrl.wkid;
    for(i = 0;i < 45;i++)
        work_out[i] = pb.intout[i];
    for(i = 0;i < 13;i++)
        work_out[45+i] = pb.ptsout.words[i];
}