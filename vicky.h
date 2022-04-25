#ifndef VICKY_H
#define VICKY_H

void vicky_init_fb(void);
void vicky_deinit_fb(void);
void vicky_get_screen_info(uint16_t *resx, uint16_t *resy, uint32_t *ncolors, uint16_t *line_length);
void vicky_set_color(uint16_t n, uint16_t red, uint16_t green, uint16_t blue);
void vicky_get_color(uint16_t n, uint16_t *red, uint16_t *green, uint16_t *blue);

#endif