/*
 * ValiantCore Kernel
 * Copyright (C) 2026 bigpower
 *
 * This program is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software
 * Foundation, version 3.
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "../include/kernel.h"
#include "../include/scheduler.h"

#define kprint_banner() kprint("--- ValiantCore OS Loading ---\n")
extern void vfs_init();
extern int rtl8111_init();
extern void net_init();


void kmain() {

   init_gdt();
   pic_init();
   init_gdt();


   kprint_banner();

   monitor_system_integrity();

   init_scheduler();
   
   vfs_init();


   net_init();
   if (rtl8111_init() == 0) {
      kprint("[KERNEL] RTL8111 driver loaded.\n");
          } else {
              kprint("[KERNEL] ERROR: RTL8111 driver failed!\n");
          }

          kprint("[KERNEL] ValiantCore ready.\n");

              while (1);
              }
