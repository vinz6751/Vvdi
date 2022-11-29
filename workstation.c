#include <stdint.h>
#include <string.h>
#include "attribute.h"
#include "config.h"
#include "font.h"
#include "utils.h"
#include "vdi.h"
#include "workstation.h"


// Drivers TODO move that somewhere else
#include "vicky.h"
#include "shifter.h"
#define DEFAULT_DRIVER    &shifter_driver


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
    int wk_id;
    for (wk_id=0; wk_id<WORKSTATIONS_SIZE; wk_id++) { // FIXME we start with one so we never return 0 on success (0 means failure). Wastes memory!
        workstation_t *wk = &workstation[wk_id];

        if (wk->in_use == true)
            continue;

        wk->in_use  = true;

        // Defaults
        vsl_type(wk_id, DEF_LINE_STYLE);
        vswr_mode(wk_id, DEF_WRT_MODE);
        vsf_perimeter(wk_id, true);
        vsf_interior(wk_id, DEF_FILL_STYLE);
        vsf_style(wk_id, DEF_FILL_STYLE);

        // Copy input settings. FIXME BROKEN, we'd need to copy words individually from corresponding workstation_settings_t props.
        memcpy(&(wk->settings), input, sizeof(uint16_t)*16);

        // Fill output
        *handle = wk_id;

        // We're a physical workstation so take care of driver stuff
        wk->driver = DEFAULT_DRIVER; // TODO when we support printers etc. ;) this will have to change
        wk->physical = true;
        if (wk->physical) {
            wk->driver->init(&wk->settings);
            wk->driver->get_features(&wk->features);
            wk->phys_wk = 0L;
        }

        screen_info_t *si = &wk->screen_info;
        wk->driver->get_screen_info(&si->max_x, &si->max_y, &si->colors, &si->line_length);

        wk->driver->get_features((workstation_features_t*)output);
        output[0] = wk->screen_info.max_x;
        output[1] = wk->screen_info.max_y;
        output[13] = wk->screen_info.colors;

        fonthead_t *default_font = font_get_loaded()[0]; // FIXME
        v_fontinit(wk_id, HIGH32(default_font), LOW32(default_font));

        if (wk->physical)
            v_clrwk(wk_id);

        return;
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
