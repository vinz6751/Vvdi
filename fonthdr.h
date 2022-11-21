// This file exists to centralise the definition of the font header,
// which was previously defined in two different places.

#ifndef FONTHDR_H
#define FONTHDR_H

#include <stdint.h>

/* font header flags */

#define F_DEFAULT   1   /* this is the default font (face and size) */
#define F_HORZ_OFF  2   /* there is a horizontal offset table */
#define F_STDFORM   4   /* the font is in standard (Motorola) format */
#define F_MONOSPACE 8   /* the font is monospaced */

/* the font header describes a font */

#define FONT_NAME_LEN 32

typedef struct font_head fonthead_t;
struct font_head {
    int16_t  font_id;
    int16_t  point;
    char     name[FONT_NAME_LEN];
    uint16_t first_ade;
    uint16_t last_ade;
    uint16_t top;
    uint16_t ascent;
    uint16_t half;
    uint16_t descent;
    uint16_t bottom;
    uint16_t max_char_width;
    uint16_t max_cell_width;
    uint16_t left_offset;          /* amount character slants left when skewed */
    uint16_t right_offset;         /* amount character slants right */
    uint16_t thicken;              /* number of pixels to smear when bolding */
    uint16_t ul_size;              /* height of the underline */
    uint16_t lighten;              /* mask for lightening  */
    uint16_t skew;                 /* mask for skewing */
    uint16_t flags;                /* see above */

    uint8_t *hor_table;            /* horizontal offsets */
    uint16_t *off_table;           /* character offsets  */
    uint16_t *dat_table;           /* character definitions (raster data) */
    uint16_t form_width;           /* width of raster in bytes */
    uint16_t form_height;          /* height of raster in lines */

    fonthead_t *next_font;           /* pointer to next font */
    uint16_t reserved;             /* Atari-reserved flag */
};

#endif /* FONTHDR_H */
