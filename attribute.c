// Management of drawing preferences

#include <stdint.h>
#include "fill_patterns.h"
#include "workstation.h"


// Local helper
static void set_fill_pattern(workstation_settings_t *settings);


// Colors ---------------------------------------------------------------------

void vs_color(int16_t handle, int16_t index, int16_t *rgb_in)
{
    workstation[handle].driver->set_color(index, rgb_in[0], rgb_in[1], rgb_in[2]);
}


int16_t vq_color(int16_t handle, int16_t index, int16_t set_flag, int16_t *rgb)
{
    workstation[handle].driver->get_color(index, &rgb[0], &rgb[1], &rgb[2]);
}


// Drawing parameters ---------------------------------------------------------

int16_t vsl_color(int16_t handle, int16_t index)
{
    workstation_t *wk = &workstation[handle];
    if (index > wk->screen_info.colors)
        index = 1;    
    wk->settings.line_color = index;
    return index;
}


int16_t vsf_color(int16_t handle, int16_t index)
{
    workstation_t *wk = &workstation[handle];
    if (index > wk->screen_info.colors)
        index = 1;    
    wk->settings.fill_color = index;
    return index;
}


int16_t vsl_type(int16_t handle, int16_t style) {
    workstation_t *wk = &workstation[handle];
    if (style > wk->features.n_line_types)
        style = LS_SOLID;
    wk->settings.line_index = style;
    return style;
}


void vsl_udsty(int16_t handle, int16_t user_defined_style) {
    workstation_t *wk = &workstation[handle];
    wk->settings.ud_ls = user_defined_style;
}


int16_t vsf_perimeter(int16_t handle, int16_t enable)
{
    workstation_t *wk = &workstation[handle];
    wk->settings.fill_perimeter = (enable != 0);
    return enable;
}


uint16_t vsf_style(uint16_t handle, uint16_t style)
{
    workstation_t *wk = &workstation[handle];

    if (wk->settings.fill_interior_style == FIS_PATTERN)
    {
        if (style > MAX_FILL_PATTERN || style < MIN_FILL_PATTERN)
            style = DEF_FILL_PATTERN;
    }
    else if (style > MAX_FILL_HATCH || style < MIN_FILL_HATCH)
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


uint16_t vswr_mode(uint16_t handle, uint16_t mode)
{
    workstation_t *wk = &workstation[handle];

    if (mode < MIN_WRT_MODE || mode > MAX_WRT_MODE)
        mode = DEF_WRT_MODE;
    wk->settings.write_mode = mode;
}


// Helps setup the fill pattern state variables
static void set_fill_pattern(workstation_settings_t *settings)
{
    uint16_t fi, pm;
    const uint16_t *pp = 0L;

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
            pp = &fill_pattern_hatchs[(fi - 8) * (pm + 1)];
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
