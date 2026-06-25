 //
 // ValiantCore Kernel
 // Copyright (C) 2026 bigpower
 // SPDX-License-Identifier: GPL-2.0-only
 //
.section "text.boot"
.global _start


_start:

    mrs x0, mpidr_el1
    and x0, x0, #0xFF
    cbz x0, master_cpu

spin:
   wfe
   b spin

master_cpu:
   ldr x1,  =_stack_top
   mov sp, x1


   ldr x1,  =__bss_start
   ldr x2,  =__bss_end
clean_bss:
   cmp x1, x2
   b.ge jump_main
   str xzr, [x1], #8
   b clean_bss

jump_main:
     bl kernel_main
     bl exceptions_init
halt:
   wfe
   b halt
    
//   ------------------------------------------
// First start on ValiantCore Aarch64 support
//  ------------------------------------------
