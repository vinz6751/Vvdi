// Atari shifter driver

#include <stdint.h>

#include "debug.h"
#include "math.h"
#include "utils.h"
#include "vdi.h"

// Shifter registers
#define PALETTE    ((uint16_t*const)0xffff8240L)
#define RESOLUTION 0xffff8260L

// Private structure for parameter passing
typedef struct
{
    uint16_t leftmask;  // left endmask
    uint16_t rightmask; // right endmask
    int16_t  width;     // line width (in WORDs)
    uint16_t *addr;     // starting screen address
} blit_parameters_t;

// Local data
static struct {
    screen_info_t screen_info; // Info about current resolution
    uint16_t v_planes_shift;
} L;

// Speed up pixel address calculations
const uint8_t shift_offset[9] = {0, 3, 2, 0, 1, 0, 0, 0, 0};

// Resolutions
const screen_info_t st_modes[] = {
    { 320, 200, 16, 160, 4 },
    { 640, 200, 4,  160, 2 },
    { 640, 400, 2,  80,  1 },
};


static void resolution_has_changed(void);
static uint16_t *get_pixel_addr(const uint16_t x, const uint16_t y);



static void init(workstation_settings_t *settings) {
    resolution_has_changed();

    settings->driver_data = (void*)&L;
}

static void get_features(workstation_features_t *features) {
    int i;
    for (i=0; i<57 ; i++)
        features->words[i] = default_capabilities.words[i];
}

static void deinit(void) {
}


static void get_screen_info(uint16_t *resx, uint16_t *resy, uint32_t *ncolors, uint16_t *line_length) {
    *resx = L.screen_info.max_x + 1;
    *resy = L.screen_info.max_y + 1;
    *ncolors = L.screen_info.colors;
    *line_length = L.screen_info.line_length;
}


#define COLOR_SCALER (1000/7)  // VDI colors are 0-1000, ST colors are 0-7. FIXME for STE it would be different (0-15)

static inline uint16_t scale1000to7(uint16_t c) {
    // This involves a division so is not really performant on a 68000...
    return c / COLOR_SCALER;
}


// Set color on LUT 0, colors are 8 bits each
static void set_color(uint16_t n, uint16_t red, uint16_t green, uint16_t blue) {
    uint16_t color;

    // ST
    color = scale1000to7(red) << 8;
    color |= scale1000to7(green) << 4;
    color |= scale1000to7(blue);
    PALETTE[n] = color;

    // TODO: STE (scale 1000 to 15)
}

// Read hardware colors and return RGB values 
static void get_color(uint16_t n, uint16_t *red, uint16_t *green, uint16_t *blue) {
    uint16_t color = PALETTE[n];

    // ST
    *blue = color & 7;
    color >> 4;
    *green = color & 7;
    color >> 4;
    *red = color & 7;
    
    /* Scale to 0-1000*/
    *blue *= COLOR_SCALER;
    *green *= COLOR_SCALER;
    *red *= COLOR_SCALER;
    // TODO: STE
}


static void resolution_has_changed(void) {
    uint32_t res = R8(RESOLUTION) & 3;

    L.screen_info = st_modes[res];
    L.v_planes_shift = shift_offset[L.screen_info.bitplanes];
    _debug("res=%ld, bitplanes = %d v_planes_shift=%d\r\n", res, L.screen_info.bitplanes, L.v_planes_shift);Cnecin();
}

static void set_pixels(const vdi_point_t *pts, uint16_t n, uint16_t color) {
    uint16_t *fb = (uint16_t*)R32(v_bas_ad);
    vdi_point_t *pt;
    int i;

    while (n--) {
        pt = (vdi_point_t *)pts++;
        uint16_t *block;
        uint16_t color2;
        uint16_t mask;
        int      plane;
        block = block = get_pixel_addr(pt->x, pt->y);
        //_debug("pixel %d,%d address: %p, bitplanes:%d\r\n", pt->x, pt->y, block, L.screen_info.bitplanes);
        mask = 0x8000 >> (pt->x & 0xf);   /* initial bit position in int16_t */
        color2 = color;

        for (plane = L.screen_info.bitplanes; plane; plane--) {
            if (color2 & 0x0001)
                *block++ |= mask;
            else
                *block++ &= ~mask;
            color2 >>= 1;
        }
    }
}


static uint16_t *get_pixel_addr(const uint16_t x, const uint16_t y)
{
    uint8_t *block;
    uint16_t x2 = x & 0xfff0;                     /* ensure that value to be shifted remains signed! */
    
    block = *((uint8_t**)v_bas_ad);               /* start of screen */
    block += x2 >> L.v_planes_shift;              /* add x coordinate part of addr */
    block += muls(y, L.screen_info.line_length);  /* add y coordinate part of addr */

    return (uint16_t*)block;
}


/*
 * set up values required by the horizontal line drawing functions
 *
 * This figures out the sizes of the left, centre, and right sections.
 * If the line lies entirely within a int16_t, then the centre and right
 * section sizes will be zero; if the line spans two WORDs, then the
 * centre size will be zero.
 * It also initialises the screen pointer.
 */
static inline void draw_rect_setup(blit_parameters_t *b, const workstation_settings_t *attr, const vdi_rectangle_t *rect)
{
    b->leftmask = 0xffff >> (rect->x1 & 0x0f);
    b->rightmask = 0xffff << (15 - (rect->x2 & 0x0f));
    b->width = (rect->x2 >> 4) - (rect->x1 >> 4) + 1;
    if (b->width == 1) {                /* i.e. all bits within 1 int16_t */
        b->leftmask &= b->rightmask;    /* so combine masks */
        b->rightmask = 0;
    }
    b->addr = get_pixel_addr(rect->x1, rect->y1);   /* init address ptr */
}

/*
 * swblit_rect_common - draw one or more horizontal lines via software
 *
 * This code does the following:
 *  1. The outermost control is via a switch() statement depending on
 *     the current drawing mode.
 *  2. Within each case, the outermost loop processes one scan line per
 *     iteration.
 *  3. Within this loop, the video planes are processed in sequence.
 *  4. Within this, the left section is processed, then the centre and/or
 *     right sections (if they exist).
 *
 * NOTE: this code seems rather longwinded and repetitive.  In fact it
 * can be shortened considerably and made much more elegant.  Doing so
 * however will wreck its performance, and this in turn will affect the
 * performance of many VDI calls.  This is not particularly noticeable
 * on an accelerated system, but is disastrous when running on a plain
 * 8MHz ST or 16MHz Falcon.  You are strongly advised not to change this
 * without a lot of careful thought & performance testing!
 */
static void draw_rectangle(const workstation_settings_t *attr, const vdi_rectangle_t *rect)
{
    const uint16_t pattern_mask = attr->pattern_mask;
    const int vplanes = L.screen_info.bitplanes;
    const int yinc = (L.screen_info.line_length >> 1) - vplanes;
    int centre, y;
    blit_parameters_t b;

    /* set up masks, width, screen address pointer */
    draw_rect_setup(&b, attr, rect);

    centre = b.width - 2 - 1;   /* -1 because of the way we construct the innermost loops */

    switch(attr->write_mode) {
    case MD_ERASE:          /* erase (reverse transparent) mode */
        for (y = rect->y1; y <= rect->y2; y++, b.addr += yinc) {
            int pattern_index = pattern_mask & y;   /* starting pattern */
            int plane;
            uint16_t color;

            for (plane = 0, color = attr->fill_color; plane < vplanes; plane++, color>>=1, b.addr++) {
                uint16_t *work = b.addr;
                uint16_t pattern = ~attr->pattern_ptr[pattern_index];
                int n;

                if (color & 0x0001) {
                    *work |= pattern & b.leftmask;  /* left section */
                    work += vplanes;
                    if (centre >= 0) {              /* centre section */
                        n = centre;
                        __asm ("1:\n\t"
                               "or.w %2,(%1)\n\t"
                               "adda.w %3,%1\n\t"
                               "dbra %0,1b" : "+d"(n), "+a"(work) : "d"(pattern),
                                              "r"(2*vplanes) : "memory", "cc");
                    }
                    if (b.rightmask) {              /* right section */
                        *work |= pattern & b.rightmask;
                    }
                } else {
                    *work &= ~(pattern & b.leftmask);   /* left section */
                    work += vplanes;
                    if (centre >= 0) {              /* centre section */
                        n = centre;
                        __asm ("1:\n\t"
                               "and.w %2,(%1)\n\t"
                               "adda.w %3,%1\n\t"
                               "dbra %0,1b" : "+d"(n), "+a"(work) : "d"(~pattern),
                                              "r"(2*vplanes) : "memory", "cc");
                    }
                    if (b.rightmask) {              /* right section */
                        *work &= ~(pattern & b.rightmask);
                    }
                }
                if (attr->multifill)
                    pattern_index += 16;                   /* advance pattern data */
            }
        }
        break;
    case MD_XOR:            /* xor mode */
        for (y = rect->y1; y <= rect->y2; y++, b.addr += yinc) {
            int pattern_index = pattern_mask & y;   /* starting pattern */
            int plane;
            uint16_t color;

            for (plane = 0, color = attr->fill_color; plane < vplanes; plane++, color>>=1, b.addr++) {
                uint16_t *work = b.addr;
                uint16_t pattern = attr->pattern_ptr[pattern_index];
                int n;

                *work ^= pattern & b.leftmask;      /* left section */
                work += vplanes;
                if (centre >= 0) {                  /* centre section */
                    n = centre;
                    __asm ("1:\n\t"
                           "eor.w %2,(%1)\n\t"
                           "adda.w %3,%1\n\t"
                           "dbra %0,1b" : "+d"(n), "+a"(work) : "d"(pattern),
                                          "r"(2*vplanes) : "memory", "cc");
                }

                if (b.rightmask) {                  /* right section */
                    *work ^= pattern & b.rightmask;
                }
                if (attr->multifill)
                    pattern_index += 16;                   /* advance pattern data */
            }
        }
        break;
    case MD_TRANS:          /* transparent mode */
_debug("y=%u, rect->y2=%u\n",y, rect->y2);
        for (y = rect->y1; y <= rect->y2; y++, b.addr += yinc) {
            int pattern_index = pattern_mask & y;   /* starting pattern */
            int plane;
            uint16_t color;

            for (plane = 0, color = attr->fill_color; plane < vplanes; plane++, color>>=1, b.addr++) {
                uint16_t *work = b.addr;
                uint16_t pattern = attr->pattern_ptr[pattern_index];
                int n;

                if (color & 0x0001) {
                    *work |= pattern & b.leftmask;  /* left section */
                    work += vplanes;
                    if (centre >= 0) {              /* centre section */
                        n = centre;
                        __asm ("1:\n\t"
                               "or.w %2,(%1)\n\t"
                               "adda.w %3,%1\n\t"
                               "dbra %0,1b" : "+d"(n), "+a"(work) : "d"(pattern),
                                              "r"(2*vplanes) : "memory", "cc");
                    }
                    if (b.rightmask) {              /* right section */
                        *work |= pattern & b.rightmask;
                    }
                } else {
                    *work &= ~(pattern & b.leftmask);   /* left section */
                    work += vplanes;
                    if (centre >= 0) {              /* centre section */
                        n = centre;
                        __asm ("1:\n\t"
                               "and.w %2,(%1)\n\t"
                               "adda.w %3,%1\n\t"
                               "dbra %0,1b" : "+d"(n), "+a"(work) : "d"(~pattern),
                                              "r"(2*vplanes) : "memory", "cc");
                    }
                    if (b.rightmask) {              /* right section */
                        *work &= ~(pattern & b.rightmask);
                    }
                }
                if (attr->multifill)
                    pattern_index += 16;                   /* advance pattern data */
            }
        }
        break;
    default:                /* replace mode */
        for (y = rect->y1; y <= rect->y2; y++, b.addr += yinc) {
            int pattern_index = pattern_mask & y;   /* starting pattern */
            int plane;
            uint16_t color;

            for (plane = 0, color = attr->fill_color; plane < vplanes; plane++, color>>=1, b.addr++) {
                uint16_t data, *work = b.addr;
                uint16_t pattern = (color & 0x0001) ? attr->pattern_ptr[pattern_index] : 0x0000;
                int n;

                data = *work & ~b.leftmask;         /* left section */
                data |= pattern & b.leftmask;
                *work = data;
                work += vplanes;
                if (centre >= 0) {                  /* centre section */
                    n = centre;
                    __asm ("1:\n\t"
                           "move.w %2,(%1)\n\t"
                           "adda.w %3,%1\n\t"
                           "dbra %0,1b" : "+d"(n), "+a"(work) : "r"(pattern),
                                          "r"(2*vplanes) : "memory", "cc");
                }
                if (b.rightmask) {                  /* right section */
                    data = *work & ~b.rightmask;
                    data |= pattern & b.rightmask;
                    *work = data;
                }
                if (attr->multifill)
                    pattern_index += 16;                   /* advance pattern data */
            }
        }
        break;
    }
}


/*
 * draw_line - draw a line (general purpose)
 *
 * This routine draws a line defined by the Line structure, using
 * Bresenham's algorithm.  The line is modified by the LN_MASK
 * variable and the wrt_mode parameter.  This routine handles
 * all interleaved-bitplane video resolutions.
 *
 * Note that for line-drawing the background color is always 0
 * (i.e., there is no user-settable background color).  This fact
 * allows coding short-cuts in the implementation of "replace" and
 * "not" modes, resulting in faster execution of their inner loops.
 *
 * This routine is more or less the one from the original VDI asm
 * code, with the following exception:
 *  . When the writing mode was XOR, and this was not the last line
 *    in a polyline, the original code decremented the x coordinate
 *    of the ending point.  This prevented polylines from xor'ing
 *    themselves at the intermediate points.  The determination of
 *    'last line or not' was done via the LSTLIN variable which was
 *    set in the polyline() function.
 * We now handle this situation as follows:
 *  . The polyline() function still sets the LSTLIN variable.  The
 *    abline() function (q.v.) adjusts the line end coordinates
 *    accordingly.
 *
 */
static void draw_line(const vdi_line_t *line, workstation_settings_t *settings, uint16_t color)
{
    uint16_t *adr;
    int16_t dx;                    /* width of rectangle around line */
    int16_t dy;                    /* height of rectangle around line */
    int16_t yinc;                  /* in/decrease for each y step */
    const int16_t xinc = L.screen_info.bitplanes; /* positive increase for each x step, planes WORDS */
    uint16_t msk;
    int plane;
    uint16_t linemask = settings->line_mask;   /* linestyle bits */

    dx = line->x2 - line->x1;
    dy = line->y2 - line->y1;

    /* calculate increase values for x and y to add to actual address */
    if (dy < 0) {
        dy = -dy;                       /* make dy absolute */
        yinc = (int32_t) -1 * L.screen_info.line_length / 2; /* sub one line of words */
    } else {
        yinc = (int32_t) L.screen_info.line_length / 2;     /* add one line of words */
    }

    adr = get_pixel_addr(line->x1, line->y1);   /* init address counter */
    msk = 0x8000 >> (line->x1&0xf);             /* initial bit position in int16_t */

    for (plane = L.screen_info.bitplanes-1; plane >= 0; plane-- ) {
        uint16_t *addr;
        int16_t  eps;              /* epsilon */
        int16_t  e1;               /* epsilon 1 */
        int16_t  e2;               /* epsilon 2 */
        int16_t  loopcnt;
        uint16_t bit;

        /* load values fresh for this bitplane */
        addr = adr;             /* initial start address for changes */
        bit = msk;              /* initial bit position in int16_t */
        linemask = settings->line_mask;

        if (dx >= dy) {
            e1 = 2*dy;
            eps = -dx;
            e2 = 2*dx;

            switch (settings->write_mode) {
            case MD_ERASE:      /* reverse transparent  */
                if (color & 0x0001) {
                    for (loopcnt=dx;loopcnt >= 0;loopcnt--) {
                        rolw1(linemask);        /* get next bit of line style */
                        if (!(linemask&0x0001))
                            *addr |= bit;
                        rorw1(bit);
                        if (bit&0x8000)
                            addr += xinc;
                        eps += e1;
                        if (eps >= 0 ) {
                            eps -= e2;
                            addr += yinc;       /* increment y */
                        }
                    }
                } else {
                    for (loopcnt=dx;loopcnt >= 0;loopcnt--) {
                        rolw1(linemask);        /* get next bit of line style */
                        if (!(linemask&0x0001))
                            *addr &= ~bit;
                        rorw1(bit);
                        if (bit&0x8000)
                            addr += xinc;
                        eps += e1;
                        if (eps >= 0 ) {
                            eps -= e2;
                            addr += yinc;       /* increment y */
                        }
                    }
                }
                break;
            case MD_XOR:        /* xor */
                for (loopcnt=dx;loopcnt >= 0;loopcnt--) {
                    rolw1(linemask);        /* get next bit of line style */
                    if (linemask&0x0001)
                        *addr ^= bit;
                    rorw1(bit);
                    if (bit&0x8000)
                        addr += xinc;
                    eps += e1;
                    if (eps >= 0 ) {
                        eps -= e2;
                        addr += yinc;       /* increment y */
                    }
                }
                break;
            case MD_TRANS:      /* or */
                if (color & 0x0001) {
                    for (loopcnt=dx;loopcnt >= 0;loopcnt--) {
                        rolw1(linemask);        /* get next bit of line style */
                        if (linemask&0x0001)
                            *addr |= bit;
                        rorw1(bit);
                        if (bit&0x8000)
                            addr += xinc;
                        eps += e1;
                        if (eps >= 0 ) {
                            eps -= e2;
                            addr += yinc;       /* increment y */
                        }
                    }
                } else {
                    for (loopcnt=dx;loopcnt >= 0;loopcnt--) {
                        rolw1(linemask);        /* get next bit of line style */
                        if (linemask&0x0001)
                            *addr &= ~bit;
                        rorw1(bit);
                        if (bit&0x8000)
                            addr += xinc;
                        eps += e1;
                        if (eps >= 0 ) {
                            eps -= e2;
                            addr += yinc;       /* increment y */
                        }
                    }
                }
                break;
            case MD_REPLACE:    /* rep */
                if (color & 0x0001) {
                    for (loopcnt=dx;loopcnt >= 0;loopcnt--) {
                        rolw1(linemask);        /* get next bit of line style */
                        if (linemask&0x0001)
                            *addr |= bit;
                        else
                            *addr &= ~bit;
                        rorw1(bit);
                        if (bit&0x8000)
                            addr += xinc;
                        eps += e1;
                        if (eps >= 0 ) {
                            eps -= e2;
                            addr += yinc;       /* increment y */
                        }
                    }
                }
                else {
                    for (loopcnt=dx;loopcnt >= 0;loopcnt--) {
                        rolw1(linemask);        /* get next bit of line style */
                        *addr &= ~bit;
                        rorw1(bit);
                        if (bit&0x8000)
                            addr += xinc;
                        eps += e1;
                        if (eps >= 0 ) {
                            eps -= e2;
                            addr += yinc;       /* increment y */
                        }
                    }
                }
            }
        } else {
            e1 = 2*dx;
            eps = -dy;
            e2 = 2*dy;

            switch (settings->write_mode) {
            case MD_ERASE:      /* reverse transparent */
                if (color & 0x0001) {
                    for (loopcnt=dy;loopcnt >= 0;loopcnt--) {
                        rolw1(linemask);        /* get next bit of line style */
                        if (!(linemask&0x0001))
                            *addr |= bit;
                        addr += yinc;
                        eps += e1;
                        if (eps >= 0 ) {
                            eps -= e2;
                            rorw1(bit);
                            if (bit&0x8000)
                                addr += xinc;
                        }
                    }
                } else {
                    for (loopcnt=dy;loopcnt >= 0;loopcnt--) {
                        rolw1(linemask);        /* get next bit of line style */
                        if (!(linemask&0x0001))
                            *addr &= ~bit;
                        addr += yinc;
                        eps += e1;
                        if (eps >= 0 ) {
                            eps -= e2;
                            rorw1(bit);
                            if (bit&0x8000)
                                addr += xinc;
                        }
                    }
                }
                break;
            case MD_XOR:        /* xor */
                for (loopcnt=dy;loopcnt >= 0;loopcnt--) {
                    rolw1(linemask);        /* get next bit of line style */
                    if (linemask&0x0001)
                        *addr ^= bit;
                    addr += yinc;
                    eps += e1;
                    if (eps >= 0 ) {
                        eps -= e2;
                        rorw1(bit);
                        if (bit&0x8000)
                            addr += xinc;
                    }
                }
                break;
            case MD_TRANS:      /* or */
                if (color & 0x0001) {
                    for (loopcnt=dy;loopcnt >= 0;loopcnt--) {
                        rolw1(linemask);        /* get next bit of line style */
                        if (linemask&0x0001)
                            *addr |= bit;
                        addr += yinc;
                        eps += e1;
                        if (eps >= 0 ) {
                            eps -= e2;
                            rorw1(bit);
                            if (bit&0x8000)
                                addr += xinc;
                        }
                    }
                } else {
                    for (loopcnt=dy;loopcnt >= 0;loopcnt--) {
                        rolw1(linemask);        /* get next bit of line style */
                        if (linemask&0x0001)
                            *addr &= ~bit;
                        addr += yinc;
                        eps += e1;
                        if (eps >= 0 ) {
                            eps -= e2;
                            rorw1(bit);
                            if (bit&0x8000)
                                addr += xinc;
                        }
                    }
                }
                break;
            case MD_REPLACE:    /* rep */
                if (color & 0x0001) {
                    for (loopcnt=dy;loopcnt >= 0;loopcnt--) {
                        rolw1(linemask);        /* get next bit of line style */
                        if (linemask&0x0001)
                            *addr |= bit;
                        else
                            *addr &= ~bit;
                        addr += yinc;
                        eps += e1;
                        if (eps >= 0 ) {
                            eps -= e2;
                            rorw1(bit);
                            if (bit&0x8000)
                                addr += xinc;
                        }
                    }
                }
                else {
                    for (loopcnt=dy;loopcnt >= 0;loopcnt--) {
                        rolw1(linemask);        /* get next bit of line style */
                        *addr &= ~bit;
                        addr += yinc;
                        eps += e1;
                        if (eps >= 0 ) {
                            eps -= e2;
                            rorw1(bit);
                            if (bit&0x8000)
                                addr += xinc;
                        }
                    }
                }
            }
        }
        adr++;
        color >>= 1;    /* shift color index: next plane */
    }
    settings->line_mask = linemask;
}



vdi_driver_t shifter_driver = {
    init,
    deinit,
    get_features,
    get_screen_info,
    set_color,
    get_color,
    resolution_has_changed,
    set_pixels,
    draw_line,
    draw_rectangle
};