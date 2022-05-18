#include <stdint.h>
#include <stdbool.h>

#include "utils.h"
#include "vdi.h"

// TODO should go in the workstation state/settings ?
static bool last_line; // (LSTLIN) indicates that line to draw is the last one. 

static bool clip_line(workstation_t *wk, vdi_line_t *line);
static int16_t clip_code(workstation_t *wk, int16_t x, int16_t y);
void abline(const vdi_line_t *line, workstation_t *wk, uint16_t color);


void polyline(workstation_t *wk, vdi_point_t *point, int count, int16_t color)
{
    int i;
    vdi_line_t line;

    for (i = count-1, last_line = false; i > 0; i--) {
        if (i == 1)
            last_line = true;
        line.x1 = point->x;
        line.y1 = point->y;
        point++;                /* advance point by point */
        line.x2 = point->x;
        line.y2 = point->y;

        if (wk->settings.clip)
            if (!clip_line(wk, &line))
                continue;

        abline(&line, wk, color);
    }
}


/*
 * clip_line - clip line if necessary
 *
 * returns FALSE iff the line lies outside the clipping rectangle
 * otherwise, updates the contents of the vdi_line_t structure & returns true
 */
static bool clip_line(workstation_t *wk, vdi_line_t *line)
{
    int16_t deltax, deltay, x1y1_clip_flag, x2y2_clip_flag, line_clip_flag;
    int16_t *x, *y;

    while ((x1y1_clip_flag = clip_code(wk, line->x1, line->y1)) |
           (x2y2_clip_flag = clip_code(wk, line->x2, line->y2))) {
        if ((x1y1_clip_flag & x2y2_clip_flag))
            return false;
        if (x1y1_clip_flag) {
            line_clip_flag = x1y1_clip_flag;
            x = &line->x1;
            y = &line->y1;
        } else {
            line_clip_flag = x2y2_clip_flag;
            x = &line->x2;
            y = &line->y2;
        }
        deltax = line->x2 - line->x1;
        deltay = line->y2 - line->y1;
        if (line_clip_flag & 1) {               /* left ? */
            *y = line->y1 + mul_div_round(deltay, (wk->settings.xmn_clip-line->x1), deltax);
            *x = wk->settings.xmn_clip;
        } else if (line_clip_flag & 2) {        /* right ? */
            *y = line->y1 + mul_div_round(deltay, (wk->settings.xmx_clip-line->x1), deltax);
            *x = wk->settings.xmx_clip;
        } else if (line_clip_flag & 4) {        /* top ? */
            *x = line->x1 + mul_div_round(deltax, (wk->settings.ymn_clip-line->y1), deltay);
            *y = wk->settings.ymn_clip;
        } else if (line_clip_flag & 8) {        /* bottom ? */
            *x = line->x1 + mul_div_round(deltax, (wk->settings.ymx_clip-line->y1), deltay);
            *y = wk->settings.ymx_clip;
        }
    }
    return true;              /* segment now clipped  */
}


/*
 * clip_code - helper function, used by clip_line()
 *
 * returns a bit mask indicating where x and y are, relative
 * to the clipping rectangle:
 *  1   x is left
 *  2   x is right
 *  4   y is above
 *  8   y is below
 */
static int16_t clip_code(workstation_t *wk, int16_t x, int16_t y)
{
    int16_t clip_flag;

    clip_flag = 0;
    if (x < wk->settings.xmn_clip)
        clip_flag = 1;
    else if (x > wk->settings.xmx_clip)
        clip_flag = 2;
    if (y < wk->settings.ymn_clip)
        clip_flag += 4;
    else if (y > wk->settings.ymx_clip)
        clip_flag += 8;
    return (clip_flag);
}

/*
 * abline - draw a line
 *
 * this is now a wrapper for the actual line drawing routines
 * OLD DOC:
 * input:
 *     line         = pointer to structure containing coordinates
 *     v_planes     = number of video planes
 *     LN_MASK      = line mask (for dashed/dotted lines)
 *     LSTLIN       = flag: TRUE iff this is the last line of a polyline
 *     write_mode   = writing mode:
 *                          0 => replace mode.
 *                          1 => or mode.
 *                          2 => xor mode.
 *                          3 => not mode.
 *
 * output:
 *     LN_MASK rotated to proper alignment with x coordinate of line end
 */
void abline(const vdi_line_t *line, workstation_t *wk, uint16_t color)
{
    vdi_line_t ordered;
    uint16_t x1,y1,x2,y2;          /* the coordinates */

#if CONF_WITH_VDI_VERTLINE
    /*
     * optimize drawing of vertical lines
     */
    if (line->x1 == line->x2) {
#if CONF_WITH_BLITTER
        if (blitter_is_enabled)
        {
            hwblit_vertical_line(line, write_mode, color);
            return;
        }
        else
#endif
        {
            vertical_line(line, write_mode, color);
            return;
        }
    }
#endif

    /* Always draw from left to right */
    if (line->x2 < line->x1) {
        /* if delta x < 0 then draw from point 2 to 1 */
        x1 = line->x2;
        y1 = line->y2;
        x2 = line->x1;
        y2 = line->y1;
    } else {
        /* positive, start with first point */
        x1 = line->x1;
        y1 = line->y1;
        x2 = line->x2;
        y2 = line->y2;
    }

    /*
     * copy a DRI kludge: if we're in XOR mode, avoid XORing intermediate
     * points in a polyline.  we do it slightly differently than DRI with
     * slightly differing results - but it's a kludge in either case.
     */
    if ((wk->settings.write_mode == MD_XOR) && !last_line)
    {
        if (x1 != x2)
            x2--;
        else if (y1 != y2)
            y2--;
    }

#if 0 // Disabled for now because draw_rect_common not strictly necessary
    /*
     * optimize drawing of horizontal lines
     */
    if (y1 == y2) {
        uint16_t linemask = wk->settings.line_mask;   /* linestyle bits */
        workstation_settings_t attr;
        vdi_rectangle_t rect;

        attr.clip = 0;
        attr.multifill = 0;
        attr.pattern_mask = 0;
        attr.pattern_ptr = &linemask;
        attr.write_mode = wk->settings.write_mode;
        attr.line_color = color;
        rect.x1 = x1;
        rect.y1 = y1;
        rect.x2 = x2;
        rect.y2 = y2;
        wk->driver->draw_rect_common(&attr, &rect);
        return;
    }
#endif    

    /*
     * draw any line
     */
    ordered.x1 = x1;
    ordered.y1 = y1;
    ordered.x2 = x2;
    ordered.y2 = y2;
    wk->driver->draw_line(&ordered, &wk->settings, color);
}
