/*
* ValiantCore Kernel
* Copyright (C) 2026 bigpower
* SPDX-License-Identifier: GPL-2.0-only
* By Finn Dev
*/
#include "../include/kernel.h"
#include "../include/terminus16.h"

#include <stdint.h>

/* PSF1 HEADER */
#define PSF1_MAGICO  0x36
#define PSF1_MAGIC1  0x04
#define PSF1_MODES12 0x01


typedef struct __attribute__ ((packed)) {
    uint8_t magic[2];
    uint8_t mode;
    uint8_t charsize;
} psf1_header_t;



static struct {
  const uint8_t   *glyphs;
  uint32_t        numglyph;
  uint32_t		  charsize;
  uint32_t        width;
  uint32_t        height;
  int             ready;
} font;

static uint32_t cursor_x = 0;
static uint32_t cursor_y = 0;
static uint32_t fg_color = 0xFFFFFF;
static uint32_t bg_color = 0x000000;

extern void      fb_but_pixel(uint32_t x, uint32_t y, uint32_t color);
extern void      fb_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);
extern uint32_t  fb_get_width(void);
extern uint32_t  fb_get_height(void);
extern int		 fb_is_ready(void);


int font_init(const uint8_t *data) {
    if (!data) return -1;


    psf1_header_t *hdr = (psf1_header_t *)data;


    if (hdr->magic[0] != PSF1_MAGICO ||
        hdr->magic[1] != PSF1_MAGIC1) {
        return -1;
    }


    font.charsize = hdr->charsize;
    font.numglyph = (hdr->mode & PSF1_MODES12) ? 512 : 256;
    font.glyphs   = data + sizeof(psf1_header_t);
    font.width    = 8;
    font.height   = hdr->charsize;
    font.ready    = 1;

    return 0;
}
int      font_is_ready(void)   { return font.ready;  }
uint32_t font_get_width(void)  { return font.width;  }
uint32_t font_get_height(void) { return font.height; }


void font_set_color(uint32_t fg, uint32_t bg) {
   fg_color = fg;
   bg_color = bg;
}

void font_set_cursor(uint32_t x, uint32_t y) {
     cursor_x = x;
     cursor_y = y;
}


void font_draw_char(uint32_t x, uint32_t y, char c, uint32_t fg, uint32_t bg, int scale) {
    if (!font.ready || !fb_is_ready()) return;

    uint32_t idx = (uint8_t)c;
    if (idx >= font.numglyph) idx = '?';

    const uint8_t *glyhp = font.glyphs + idx * font.charsize;

    for (uint32_t row = 0; row < font.height; row++) {
        uint8_t bits = glyhp[row];
        for (uint32_t col = 0; col < font.width; col++) {
            uint32_t color = (bits & (0x80 >> col)) ? fg : bg;
            for (int sy = 0; sy < scale; sy++) {
               for (int sx = 0; sx < scale; sx++) {
                   fb_but_pixel(x + col * scale + sx,
                                y + row * scale + sy,
                                color);
                          }
                     }
                 }
           }
       }

void font_draw_string(uint32_t x, uint32_t y, const char *s, uint32_t fg, uint32_t bg, int scale) {

   if (!s) return;
   uint32_t cx = x;

   while (*s) {
     if (*s == '\n') {
         cx  = x;
         y += font.height	 * scale;
      } else {
          font_draw_char(cx, y, *s, fg, bg, scale);
          cx += font.width * scale;
       }
       s++;
     }
     
}
              

void font_draw_string_centered(uint32_t y, const char *s, uint32_t fg, uint32_t bg, int scale) {
    if (!s) return;
    int len = 0;
    while (s[len]) len++;
    uint32_t tw = (uint32_t)len * font.width * scale;
    uint32_t x  = (fb_get_width() > tw) ? (fb_get_width() - tw) / 2 : 0;
    font_draw_string(x, y, s, fg, bg, scale);
}


/* ValiantCore By Finn Dev */
void kprint(char *s) {
   if (!s || !fb_is_ready() || !font.ready) return;

   uint32_t char_w = font.width;
   uint32_t char_h = font.height;
   uint32_t cols   = fb_get_width() / char_w;
   uint32_t rows   = fb_get_height() / char_h;

   while (*s) {
      if (*s == '\n') {
         cursor_x = 0;
         cursor_y++;
      } else if (*s == '\b') {
         if (cursor_x > 0) {
            cursor_x--;
            font_draw_char(cursor_x * char_w, cursor_y * char_h, ' ', fg_color, bg_color, 1);
         }
      } else {
         font_draw_char(cursor_x * char_w, cursor_y * char_h, *s, fg_color, bg_color, 1);
         cursor_x++;
         if (cursor_x >= cols) {
            cursor_x = 0;
            cursor_y++;
         }
      }

      if (cursor_y >= rows) {
         fb_fill_rect(0, 0, fb_get_width(), fb_get_height(), bg_color);
         cursor_x = 0;
         cursor_y = 0;
      }
      s++;
   }
}

void kprint_color(char *s, uint32_t fg, uint32_t bg) {
   uint32_t old_fg = fg_color;
   uint32_t old_bg = bg_color;
   fg_color = fg;
   bg_color = bg;
   kprint(s);
   fg_color = old_fg;
   bg_color = old_bg;
}

void kprint_at(uint32_t x, uint32_t y,  const char *s, uint32_t fg, uint32_t bg, int scale) {
      font_draw_string(x, y, s, fg, bg, scale);
}
