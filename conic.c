/*
 * fVDI circle/ellipse/pie/arc code
 *
 * Copyright 1999/2001-2003, Johan Klockars
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 *
 * This is extracted and modified from code with an
 * original copyright as follows.
 */

/*************************************************************************
**       Copyright 1999, Caldera Thin Clients, Inc.                     **
**       This software is licenced under the GNU Public License.        **
**       Please see LICENSE.TXT for further information.                **
**                                                                      **
**                  Historical Copyright                                **
**                                                                      **
**  Copyright (c) 1987, Digital Research, Inc. All Rights Reserved.     **
**  The Software Code contained in this listing is proprietary to       **
**  Digital Research, Inc., Monterey, California and is covered by U.S. **
**  and other copyright protection.  Unauthorized copying, adaptation,  **
**  distribution, use or display is prohibited and may be subject to    **
**  civil and criminal penalties.  Disclosure to others is prohibited.  **
**  For the terms and conditions of software code use refer to the      **
**  appropriate Digital Research License Agreement.                     **
**                                                                      **
**************************************************************************/

#include <stdint.h>

#include "fill_patterns.h"
#include "helper_structs.h"
#include "math.h"
#include "memory.h"
#include "vdi.h"

#define MAX_ARC_CT 256

#define ARC_SPLIT 16384  /* 1/4 as many lines as largest ellipse axel radius in pixels */
#define ARC_MIN   16     /* Minimum number of lines in an ellipse */
#define ARC_MAX   256    /* Maximum */

#define GDP_ARC                 2
#define GDP_PIE_SLICE           3
#define GDP_CIRCLE              4
#define GDP_ELLIPSE             5
#define GDP_ELLIPITICAL_ARC     6
#define GDP_ELLIPTICAL_PIE      7
#define GDP_ROUNDED_RECT        8
#define GDP_FILLED_ROUNDED_RECT 9

/* x*y/z*/
#define SMUL_DIV(x,y,z) ((int16_t)(((int16_t)(x)*(int32_t)((int16_t)(y)))/(int16_t)(z)))


static int calc_number_of_steps(int32_t x_radius, int32_t y_radius);
static void col_pat(const workstation_t *vwk, Fgbg *fill_colour, Fgbg *border_colour, int16_t **pattern);
static void calc_arc(
    const workstation_t *vwk, int16_t gdp_code,
    int32_t xc, int32_t yc,
    int32_t x_radius, int32_t y_radius,
    int32_t beg_ang, int32_t end_ang,
    int32_t delta_angle, int32_t n_steps,
    Fgbg fill_colour, Fgbg border_colour,
    int16_t *pattern, int16_t *points, int32_t mode, int32_t interior_style);


void ellipsearc(
    workstation_t *vwk, int32_t gdp_code,
    int32_t xc, int32_t yc,
    int32_t x_radius, int32_t y_radius,
    int32_t beg_ang, int32_t end_ang)
{
    int delta_angle, n_steps;
    int16_t *points, *pattern;
    Fgbg fill_colour, border_colour;
    int32_t interior_style;

    delta_angle = (int)(end_ang - beg_ang);
    if (delta_angle <= 0)
        delta_angle += 3600;

    n_steps = calc_number_of_steps(x_radius, y_radius);
    n_steps = SMUL_DIV(delta_angle, n_steps, 3600);
    if (n_steps == 0)
        return;

    if ((points = (int16_t *)mem_allocate(0)) == NULL)
        return;

    pattern = 0;
    interior_style = 0;
    border_colour = vwk->settings.line_color;
    if (gdp_code == GDP_ELLIPTICAL_PIE || gdp_code == GDP_ELLIPSE)
    {
        col_pat(vwk, &fill_colour, &border_colour, &pattern);
        interior_style = ((int32_t) vwk->settings.fill_interior_style << 16) | (vwk->settings.fill_pattern_hatch_style & 0xffffL);
    }

    /* Dummy fill colour, pattern and interior style since not filled */
    calc_arc(vwk, gdp_code, xc, yc, x_radius, y_radius, beg_ang, end_ang, delta_angle,
            n_steps, fill_colour, border_colour, pattern, points, vwk->settings.write_mode, interior_style);

    mem_free(points);
}


void rounded_box(workstation_t *vwk, int16_t gdp_code, vdi_rectangle_t *coords)
/* int32_t x1, int32_t y1, int32_t x2, int32_t y2) */
{
    int16_t i, j;
    int16_t rdeltax, rdeltay;
    int16_t xc, yc, x_radius, y_radius;
    workstation_t *wk = vwk->phys_wk;
    int16_t *points, *pattern;
    Fgbg fill_colour, border_colour;
    int32_t interior_style;

    if ((points = (int16_t*)mem_allocate(0)) == NULL)
        return;

    pattern = 0;
    interior_style = 0;
    border_colour = vwk->settings.line_color;
    if (gdp_code == GDP_FILLED_ROUNDED_RECT)
    {
        col_pat(vwk, &fill_colour, &border_colour, &pattern);

        interior_style = ((int32_t) vwk->settings.fill_interior_style << 16) | (vwk->settings.fill_pattern_hatch_style & 0xffffL);
    }

    // Could use sort_corners if x1/x2/y1/y2 where in a vdi_rectangle_t
    vdi_rectangle_t c = *coords;
    sort_corners(&c);

    rdeltax = (c.x2 - c.x1) / 2;
    rdeltay = (c.y2 - c.y1) / 2;

    x_radius = wk->screen_info.mfdb.width >> 6;
    if (x_radius > rdeltax)
        x_radius = rdeltax;

    y_radius = SMUL_DIV(x_radius, wk->screen_info.pixel.width, wk->screen_info.pixel.height);
    if (y_radius > rdeltay)
    {
        y_radius = rdeltay;
        x_radius = SMUL_DIV(y_radius, wk->screen_info.pixel.height, wk->screen_info.pixel.width);
    }
    y_radius = -y_radius;

    /* n_steps = calc_number_of_steps(x_radius, y_radius); */

    for (i = 0; i < 5; i++)
    {
        points[i * 2] = SMUL_DIV(Icos(900 - 225 * i), x_radius, 32767);
        points[i * 2 + 1] = SMUL_DIV(Isin(900 - 225 * i), y_radius, 32767);
    }

    xc = c.x2 - x_radius;
    yc = c.y1 - y_radius;
    j = 10;
    for (i = 9; i >= 0; i--)
    {
        points[j + 1] = yc + points[i--];
        points[j] = xc + points[i];
        j += 2;
    }
    xc = c.x1 + x_radius;
    j = 20;
    for (i = 0; i < 10; i++)
    {
        points[j++] = xc - points[i++];
        points[j++] = yc + points[i];
    }
    yc = c.y2 + y_radius;
    j = 30;
    for (i = 9; i >= 0; i--)
    {
        points[j + 1] = yc - points[i--];
        points[j] = xc - points[i];
        j += 2;
    }
    xc = c.x2 - x_radius;
    j = 0;
    for (i = 0; i < 10; i++)
    {
        points[j++] = xc + points[i++];
        points[j++] = yc - points[i];
    }
    points[40] = points[0];
    points[41] = points[1];

    if (gdp_code == GDP_ROUNDED_RECT)
    {
        c_pline(vwk, 21, border_colour, points);
    } else
    {
        fill_poly(vwk, points, 21, fill_colour, pattern, &points[42], vwk->settings.write_mode, interior_style);
        if (vwk->settings.fill_perimeter)
            c_pline(vwk, 21, border_colour, points);
    }

    mem_free(points);
}


static int calc_number_of_steps(int32_t x_radius, int32_t y_radius)
{
    int32_t n_steps;

    n_steps = x_radius > y_radius ? x_radius : y_radius;
    n_steps = (n_steps * ARC_SPLIT) >> 16;

    if (n_steps < ARC_MIN)
        n_steps = ARC_MIN;
    else if (n_steps > ARC_MAX)
        n_steps = ARC_MAX;

    return (int)n_steps;
}


static void col_pat(const workstation_t *vwk, Fgbg *fill_colour, Fgbg *border_colour, int16_t **pattern)
{
    const int16_t interior = vwk->settings.fill_interior_style; /* Convenience */

    *border_colour = vwk->settings.fill_color;
    
    if (interior != FIS_HOLLOW)
        *fill_colour = vwk->settings.fill_color;
    else {
        fill_colour->background = vwk->settings.fill.colour.foreground;
        fill_colour->foreground = vwk->settings.fill.colour.background;
    }

    if (interior == FIS_USERDEF) {
        *pattern = vwk->settings.fill.user.pattern.in_use;
    }
    else {
        /* In all cases we return a pattern of 16 words */
        *pattern = fill_patterns_by_interior[interior];
        if (interior & (FIS_PATTERN|FIS_HATCH))
        {
            /* Select pattern from the collection */
            *pattern += (vwk->settings.fill_pattern_hatch_style - 1) * 16/*each pattern is 16 words*/;
        }
    }
}


static void calc_arc(
    const workstation_t *vwk, int16_t gdp_code,
    int32_t xc, int32_t yc,
    int32_t x_radius, int32_t y_radius,
    int32_t beg_ang, int32_t end_ang, int32_t delta_angle, int32_t n_steps,
    Fgbg fill_colour, Fgbg border_colour,
    int16_t *pattern, int16_t *points, int32_t mode, int32_t interior_style)
{
    int16_t i, j, start, angle;

    /* if (vwk->clip.on) */
    {
        if (((xc + x_radius) < vwk->settings.clip_rect.xmin) ||
            ((xc - x_radius) > vwk->settings.clip_rect.xmax) ||
            ((yc + y_radius) < vwk->settings.clip_rect.ymin) ||
            ((yc - y_radius) > vwk->settings.clip_rect.ymax))
            return;
    }
    start = angle = beg_ang;

    *points++ = SMUL_DIV(Icos(angle), x_radius, 32767) + xc;
    *points++ = yc - SMUL_DIV(Isin(angle), y_radius, 32767);

    for (i = 1, j = 2; i<n_steps; i++, j += 2)
    {
        angle = SMUL_DIV(delta_angle, i, n_steps) + start;
        *points++ = SMUL_DIV(Icos(angle), x_radius, 32767) + xc;
        *points++ = yc - SMUL_DIV(Isin(angle), y_radius, 32767);
    }
    angle = end_ang;

    *points++ = SMUL_DIV(Icos(angle), x_radius, 32767) + xc;
    *points++ = yc - SMUL_DIV(Isin(angle), y_radius, 32767);

    /*
     * If pie wedge, draw to center and then close.
     * If arc or circle, do nothing because loop should close circle.
     */

    if (gdp_code == GDP_PIE_SLICE || gdp_code == GDP_ELLIPTICAL_PIE)
    {
        /* Pie wedge */
        n_steps++;
        *points++ = xc;
        *points++ = yc;
    }

    if (gdp_code == GDP_ARC || gdp_code == GDP_ELLIPITICAL_ARC) /* Open arc */
    {
        c_pline(vwk, n_steps + 1, border_colour, points - (n_steps + 1) * 2);
    }
    else
    {
        fill_poly(vwk, points - (n_steps + 1) * 2, n_steps + 1, fill_colour, pattern, points, mode, interior_style);
        
        if (gdp_code != GDP_CIRCLE && gdp_code != GDP_ELLIPSE) /* TOS VDI doesn't draw the perimeter for v_circle() and v_ellipse() */
            if (vwk->settings.fill_perimeter)
                c_pline(vwk, n_steps + 1, border_colour, points - (n_steps + 1) * 2);
    }
}
