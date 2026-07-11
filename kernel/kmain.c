/*
 * ValiantCore Kernel
 * Copyright (C) 2026 bigpower
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include "../include/kernel.h"
#include "../include/scheduler.h"
#include "../include/font.h"
#include "../include/framebuffer.h"
#include "../include/multiboot2.h"

#define kprint_banner() kprint("--- ValiantCore OS Loading ---\n")
extern void vfs_init();
extern int rtl8111_init();
extern void net_init();


void kmain() {

   init_gdt();     // By Finn Dev
   pic_init();     // 
   pit_init(1000); // 
   init_idt();     //
   init_scheduler(); 

   kprint_banner();

   monitor_system_integrity();
   
   fat32_init();
   vfs_init();

   

font_init(Uni2_Terminus16_psf);   
font_set_color(0xFFFFFF, 0x0A0A2A);  
fb_clear(0x0A0A2A);


   net_init();
   if (rtl8111_init() == 0) {
      kprint("[KERNEL] RTL8111 driver loaded.\n");
          } else {
              kprint("[KERNEL] ERROR: RTL8111 driver failed!\n");
          }

          kprint("[KERNEL] ValiantCore ready.\n");

         mb2_tag_framebuffer_t *fb_tag =
    (mb2_tag_framebuffer_t *)mb2_find_tag(mb2_info, MB2_TAG_FRAMEBUFFER);

if (fb_tag) {
    fb_init(fb_tag->framebuffer_addr,
            fb_tag->framebuffer_pitch,
            fb_tag->framebuffer_width,
            fb_tag->framebuffer_height,
            fb_tag->framebuffer_bpp);
    
}

              while (1);
              }
