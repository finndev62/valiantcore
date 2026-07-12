/*
* ValiantCore Kernel
* Copyright (C) 2026 bigpower
* SPDX-License-Identifier: GPL-2.0-only
* By Finn Dev
*/

#include "../include/kernel.h"
#include <stdint.h>

static volatile uint8_t *fb_addr   = 0;
static uint32_t fb_pitch  = 0;
static uint32_t fb_width  = 0;
static uint32_t fb_height = 0;
static uint8_t  fb_bpp    = 0;
static int      fb_ready  = 0;

int fb_init(uint64_t addr, uint32_t pitch, uint32_t width, uint32_t height, uint8_t bpp) {
   if (!addr) return -1;


   fb_addr    = (volatile uint8_t *)(addr_t)addr;
   fb_pitch   = pitch;
   fb_width   = width;
   fb_height  = height;
   fb_bpp     = bpp;
   fb_ready	  = 1;
   
    kprint("[FBI] Framebuffer Initialized\n");
    return 0;
}

int      fb_is_ready(void)     { return fb_ready; }
uint32_t fb_get_width(void)    { return fb_width; }
uint32_t fb_get_height(void)   { return fb_height; }



void fb_but_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (!fb_ready) return;
    if (x >= fb_width || y >= fb_height) return;


    uint32_t offset = y * fb_pitch + x * (fb_bpp / 8);
    volatile uint32_t *pixel = (volatile uint32_t *)(fb_addr + offset);
    *pixel = color;
}


void fb_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {
    if (!fb_ready) return;

    for (uint32_t row = 0; row < h; row++) {
        uint32_t offset = (y + row) * fb_pitch + x * (fb_bpp / 8);
        volatile uint32_t *line = (volatile uint32_t *)(fb_addr + offset);
        for (uint32_t col = 0; col < w; col++) {
            line[col] = color;
        }
    }
}


void fb_clear(uint32_t color) {
      if (!fb_ready) return;
      fb_fill_rect(0, 0, fb_width, fb_height, color);
}
/*----ValiantCore----*/
void fb_draw_line(int x0, int y0, int x1, int y1, uint32_t color) {
     if (!fb_ready) return;

     int dx  =  (x1 > x0) ? (x1 - x0) : (x0 - x1);
     int dy  = -((y1 > y0) ? (y1 - y0) : (y0 - y1));
     int sx  =  (x0 < x1) ? 1 : -1;
     int sy  =  (y0 < y1) ? 1 : -1;
     int err = dx + dy;
             
     while (1) {
          fb_but_pixel((uint32_t)x0, (uint32_t)y0, color);
          if (x0 == x1 && y0 == y1) break;
          int e2 = 2 * err;
          if (e2 >= dy) { err += dy; x0 += sx; }
          if (e2 <= dx) { err += dx; y0 += sy; }
     }
}

void fb_draw_circle(int cx, int cy, int r, uint32_t color) {
   if (!fb_ready) return;

   int x = r, y = 0, err = 0;

   while (x >= y) {
      fb_but_pixel(cx + x, cy + y, color);
      fb_but_pixel(cx + y, cy + x, color);
      fb_but_pixel(cx -	y, cy + x, color);
      fb_but_pixel(cx - x, cy + y, color);
      fb_but_pixel(cx - x, cy - y, color);
      fb_but_pixel(cx - y, cy - x, color);
      fb_but_pixel(cx + y, cy - x, color);
      fb_but_pixel(cx + x, cy - y, color);

      y++;
      err += 1 + 2 * y;
      if (2 *(err - x) + 1 > 0) {
         x--;
         err += 1 - 2 * x;
      }
   }
}

/* __      __  _______  _       ___  _______  __    _  _______  _______  _______  ______   _______ 
*|  |    |  ||   _   || |     |   ||   _   ||  |  | ||       ||       ||       ||    _  | |       |
*|  |    |  ||  |_|  || |     |   ||  |_|  ||   |_| ||_     _||       ||       ||   |_| | |    ___|
*|  |    |  ||       || |     |   ||       ||       |  |   |  |       ||       ||    _  | |   |___ 
* \  \  /  / |       || |___  |   ||       ||  _    |  |   |  |      _||       ||   | \  \|    ___|
*  \  \/  /  |   _   ||       ||   ||   _   || | |   |  |   |  |     |_ |       ||   |  |  ||   |___ 
*   \____/   |__| |__||_______||___||__| |__||_|  |__|  |___|  |_______||_______||___|  |__||_______|
*/
void fb_fill_circle(int cx, int cy, int r, uint32_t color) {
    if (!fb_ready) return;

     for (int y = -r; y <= r; y++) {
        for (int x = -r; x <= r; x++) {
            if (x * x + y * y <= r * r)
               fb_but_pixel((uint32_t)(cx + x), (uint32_t)(cy + y), color);
            }
        }
 }
 /* --------- The End --------- */

