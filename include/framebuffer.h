#ifndef FRAMEBUFFER_H
#define FRAMBUFFER_H

#include <stdint.h>

int fb_init(uint64_t addr, uint32_t pitch, uint32_t width, uint32_t height, uint8_t bpp);
int fb_is_ready(void);
uint32_t fb_get_width(void);
uint32_t fb_get_height(void);
void fb_put_pixel(uint32_t x, uint32_t y, uint32_t color);
void fb_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t, uint32_t h, uint32_t color);
void fb_clear(void);
void     fb_draw_line(int x0, int y0, int x1, int y1, uint32_t color);
void     fb_draw_circle(int cx, int cy, int r, uint32_t color);
void     fb_fill_circle(int cx, int cy, int r, uint32_t color);

#endif /* FRAMBUFFER_H */
