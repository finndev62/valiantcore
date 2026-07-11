#ifndef FONT_H
#define FONT_H

#include <stdint.h>

int      font_init(const uint8_t *font_data);
int      font_is_ready(void);
uint32_t font_get_width(void);
uint32_t font_get_height(void);
void     font_set_color(uint32_t fg, uint32_t bg);
void     font_draw_char(uint32_t x, uint32_t y, char c, uint32_t fg, uint32_t bg, int scale);
void     font_draw_string(uint32_t x, uint32_t y, const char *s, uint32_t fg, uint32_t bg, int scale);
void kprint(const char *s);
void kprint_color(const char *s, uint32_t fg, uint32_t bg);
void kprint_at(uint32_t x, uint32_t y, const char *s, uint32_t fg, uint32_t bg, int scale);

#endif /* FONT_H */
