/* Text functions */

#include <stdint.h>
#include <osbind.h>

#include "debug.h"
#include "font.h"
#include "vdi.h"
#include "text.h"

#define POINTS_BUF_SIZE 16*8 *2 /* Must be at least twice the size of the largest ever drawable character cell */
static vdi_point_t fg_points[POINTS_BUF_SIZE];

/* This is a "parameter (input and return results) block which exists only for performance reasons to speed up
 * the passing and return of information from v_gtext to render_char. */
static struct text_out_ctx_t {
    char c;
    uint16_t x;
    uint16_t y;
    uint16_t text_color;
    uint16_t form_height;
    uint16_t form_width;
    uint16_t fg_point; 
    fonthead_t *font;
    /* Returned by v_gchar */
    uint16_t rendered_cell_width;
} ctx;

static void render_char(void); /* Actually uses struct text_out_ctx_t for parameter/return values. */


int16_t vst_color(int16_t handle, int16_t color_index) {
    workstation_t *wk = &workstation[handle];
    if (color_index > wk->features.n_colors - 1)
        color_index = 1;
    wk->settings.text_color = color_index;
}


int16_t vst_font(int16_t handle, int16_t font_id) {
    workstation_t *wk = &workstation[handle];
    fonthead_t *font = font_get_by_id(font_id);
    
    wk->settings.cur_font = font ? font : font_get_loaded()[0];
    return wk->settings.cur_font->font_id;
}


int16_t vst_effects(int16_t handle, int16_t effect) {
    workstation_t *wk = &workstation[handle];
    wk->settings.text_style = effect & TEXT_FONT_SUPPORTED_EFFECTS;

    return wk->settings.text_style;
}

void vst_height(int16_t handle, int16_t height, int16_t *char_width, int16_t *char_height, int16_t *cell_width, int16_t *cell_height) {
    workstation_t *wk = &workstation[handle];
    /* TODO This implementation is not efficient, and we don't support scaling. */

    fonthead_t *selected_font = wk->settings.cur_font;

    for (int i=0; i<MAX_LOADED_FONTS; i++) {
        fonthead_t *font = font_find_by_id_and_height(wk->settings.cur_font->font_id, height);
        if (font) {
            selected_font = font;
            goto do_return;
        }

        /* Try a smaller one. */
        height--;

        if (height <= 5)
            break; /* Give up, and for performance reasons. */
    }

    /* We didn't find anything, so keep the current one. */
    selected_font = wk->settings.cur_font;

do_return:
    *char_width = selected_font->max_char_width;
    *char_height = selected_font->top;
    *cell_width = selected_font->max_cell_width;
    *cell_height = selected_font->top - selected_font->bottom + 1;
}


void v_gtext(int16_t handle, vdi_point_t xy, uint16_t *string) {
    workstation_t *wk = &workstation[handle];
    fonthead_t *font = wk->settings.cur_font; 
    void (*render)(const vdi_point_t *pts, uint16_t n, uint16_t color) = wk->driver->set_pixels;

    /* Setup the context/parameter block we use to talk to render_char */
    ctx.font = font;
    ctx.form_height = font->form_height;
    ctx.form_width = font->form_width;
    ctx.text_color = 1;//wk->settings.text_color;
    ctx.fg_point = 0;
    ctx.x = xy.x;
    ctx.y = xy.y;

    /* Must have space for the largest drawable cell to render*/
    int threshold = POINTS_BUF_SIZE - font->max_cell_width * font->form_height;

    for (uint16_t *c = string; *c; c++) {
        ctx.c = (char)*c;

        render_char(/*&ctx is implicit so to say*/);

        /* Advance for next char to draw */
        ctx.x += ctx.rendered_cell_width; 

        if (ctx.fg_point >= threshold) {
            /* Buffer of points is full, so render them and reset the buffer. */
            render(fg_points, ctx.fg_point, ctx.text_color);
            ctx.fg_point = 0;
        }
    }

    if (ctx.fg_point != 0) {
        render(fg_points, ctx.fg_point, ctx.text_color);
        ctx.fg_point = 0;
    }
}


static void render_char(void) {
    uint8_t *src = font_char_address(ctx.font, ctx.c);
    if (src == 0L) {
        ctx.rendered_cell_width = 3;
        return;
    }

    /* If wanted to do transformations on the text, we'd do it here and store the result. */

    ctx.rendered_cell_width = font_char_width(ctx.font, ctx.c); /* FIXME should be calculated using the font's offset values. */

    /* TODO: support different write modes, then effects... */
    int ix;
    int iy = ctx.y;
    for (int r = 0; r < ctx.form_height; r++) {
        ix = ctx.x;
        uint32_t mask = 1 << ctx.rendered_cell_width; /* Pixel selector. FIXME this limits us to 32 pixels-wide fonts. */
        while (mask) {
            if (*src & mask) {
                fg_points[ctx.fg_point].x = ix;
                fg_points[ctx.fg_point].y = iy;
                ctx.fg_point++;
                /* TODO should check if the points array is full, if so should flush to screen. 
                 * We don't check because our caller makes sure the buffer is large enough to render a full cell. */
            }
            ix++;
            mask >>= 1;
        }
        src += ctx.form_width;
        iy++;
    }
}
