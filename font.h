#ifndef FONT_H
#define FONT_H

#include <stdbool.h>
#include <stdint.h>
#include "fonthdr.h"

#define MAX_LOADED_FONTS 10

void font_init(void);
void font_deinit(void);
bool font_register(fonthead_t *font);
fonthead_t *font_get_by_id(int16_t font_id);
fonthead_t *font_find_by_id_and_height(int16_t font_id, int16_t height);
fonthead_t **font_get_loaded(void);

int16_t vqt_name(int16_t handle, int16_t element_num, int16_t *name);

void v_fontinit(int16_t handle, int16_t fh_high, int16_t fh_low);

uint8_t *font_char_address(fonthead_t const *font, char ascii);
uint16_t font_char_width(fonthead_t const *font, uint16_t ascii);
fonthead_t *font_load(const char *filename);

#endif
