/* Support for fonts */

#include <osbind.h>
#include <stdbool.h>
#include <stdint.h>

#include "debug.h"
#include "font.h"
#include "fonthdr.h"
#include "utils.h"
#include "vdi.h"


/* Fonts are in Intel format */
static void fix_font_endianness(fonthead_t *font);

static fonthead_t *loaded_fonts[MAX_LOADED_FONTS];


/* Initialize the font system */
void font_init(void) {
    _debug("font_init\r\n");
    for (int i=0; i<MAX_LOADED_FONTS; i++)
        loaded_fonts[i] = 0L;

    fonthead_t *font = font_load("c:\\gemsys\\MONACO10.FNT");
}


bool font_register(fonthead_t *font) {
    _debug("font_register\r\n");
    for (int i=0; i<MAX_LOADED_FONTS; i++) {
        _debug("*\r\n");
        if (loaded_fonts[i] == 0L) {
            loaded_fonts[i] = font;
            return true;
        }
    }
    return false;
}


void font_deinit(void) {
    for (int i=0 ; i<MAX_LOADED_FONTS; i++) {
        if (loaded_fonts[i] != 0L) {
            Mfree(loaded_fonts[i]);
            loaded_fonts[i] = 0L;
        }
    }
}


fonthead_t **font_get_loaded(void) {
    _debug("font_get_loaded\r\n");
    return loaded_fonts;
}


fonthead_t *font_get_by_id(int16_t font_id) {
    for (int i=0; i<MAX_LOADED_FONTS; i++) {
        fonthead_t *font = loaded_fonts[i];
        if (font != 0L && font->font_id == font_id)
            return font;
    }
    return 0L;
}


fonthead_t *font_find_by_id_and_height(int16_t font_id, int16_t height)
{
    /* TODO callers should do differently, this is not very efficient. */
    for (int i=0; i<MAX_LOADED_FONTS; i++) {
        fonthead_t *font = loaded_fonts[i];
        if (font != 0L && font->font_id == font_id && font->top == height)
            return font;
    }
    return 0L;
}


void v_fontinit(int16_t handle, int16_t fh_high, int16_t fh_low) {
    workstation_t *wk = &workstation[handle];

    if (wk->settings.loaded_fonts)
        return;

    fonthead_t *font = (fonthead_t*)(MAKE_UINT32(fh_high, fh_low));

    wk->settings.loaded_fonts = font;
    font->next_font = 0L;

    wk->settings.cur_font = font;
}


fonthead_t *font_load(const char *filename) {
    fonthead_t *font;
    int32_t size;

    if (util_read_file_to_memory(filename, (void**)&font, &size) < 0)
        return 0L;

    fix_font_endianness(font);

    /* Fix up address of tables */
    uint32_t file_data = (uint32_t)font;
    font->hor_table += file_data;
    font->off_table += file_data / sizeof(uint16_t);
    font->dat_table += file_data / sizeof(uint16_t);

    font_register(font);

    return font;
}


static void fix_font_endianness(fonthead_t *font)
{
    swap16(font->font_id);
    swap16(font->point);
    swap16(font->first_ade);
    swap16(font->last_ade);
    swap16(font->top);
    swap16(font->ascent);
    swap16(font->half);
    swap16(font->descent);
    swap16(font->bottom);
    swap16(font->max_char_width);
    swap16(font->max_cell_width);
    swap16(font->left_offset);
    swap16(font->right_offset);
    swap16(font->thicken);
    swap16(font->ul_size);
    swap16(font->lighten);
    swap16(font->skew);
    swap16(font->flags);
    swap16(font->form_width);
    swap16(font->form_height);
    
    const int nchars = font->last_ade - font->first_ade + 1;

    swap32(font->hor_table);
    swap32(font->off_table);
    swap32(font->dat_table);

    uint32_t base_address = (uint32_t)font;
    /* CAVEAT: The font MUST be loaded at an even address. */
    uint16_t *off_table = font->off_table + base_address / sizeof(uint16_t);
    uint16_t *dat_table = font->dat_table + base_address / sizeof(uint16_t);

    for (int i=0; i<nchars; i++) {
        swap16(off_table[i]);
        swap16(dat_table[i]);
    }
}


uint16_t font_char_width(fonthead_t const *font, uint16_t ascii) {
    ascii -= font->first_ade;
    return (font->off_table[ascii+1] - font->off_table[ascii]);
}


uint8_t *font_char_address(fonthead_t const *font, char ascii)
{
    uint16_t offset;

    if (ascii < font->first_ade || ascii > font->last_ade) {
        return 0L; /* ASCII code not in range of what the font contains. */
    }

    offset = ascii - font->first_ade;
    offset >>= 3; /* Convert from pixels to bytes, FIXME this assumes 8 pixels wide monospaced font. */

    return &((uint8_t*)(font->dat_table))[font->off_table[ascii - font->first_ade] >> 3];
}


int16_t vqt_name(int16_t handle, int16_t element_num, int16_t *name) {
    workstation_t *wk = &workstation[handle];

    if (element_num < 0 || element_num >= MAX_LOADED_FONTS)
        return 0;

    fonthead_t *font = loaded_fonts[element_num];
    if (font == 0L)
        return 0;

    /* Copy the name (char* to int16_t*) */
    for (int i=0; i<FONT_NAME_LEN; i++)
        name[i] = font->name[i];

    return font->font_id;
}
