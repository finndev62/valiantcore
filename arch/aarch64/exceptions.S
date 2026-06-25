// Table layout (most commonly used for EL1):
//   Offset  Source                  Tip
//   0x000   Current EL, SP0         Synchronous
//   0x080   Current EL, SP0         IRQ
//   0x100   Current EL, SP0         FIQ
//   0x180   Current EL, SP0         SError
//   0x200   Current EL, SPx         Synchronous   <- kernel bunu kullanir
//   0x280   Current EL, SPx         IRQ           <- kernel bunu kullanir
//   0x300   Current EL, SPx         FIQ
//   0x380   Current EL, SPx         SError
//   0x400   Lower EL, AArch64       Synchronous
//   0x480   Lower EL, AArch64       IRQ
//   0x500   Lower EL, AArch64       FIQ
//   0x580   Lower EL, AArch64       SError
//   0x600   Lower EL, AArch32       Synchronous
//   0x680   Lower EL, AArch32       IRQ
//   0x700   Lower EL, AArch32       FIQ
//   0x780   Lower EL, AArch32       SError
// ------------------------------------------------------------
//
// ValiantCore Kernel
// Copyright (C) 2026 bigpower
// SPDX-License-Identifier: GPL-2.0-only
// ------------------------------------------------------------
.section ".text.exceptions"
.global vector_table_e1
.global exceptions_init

.extern sync_handler
.extern irq_handler_aarch64
.extern fiq_handler_aarch64
.extern serror_handler


.macro SAVE_AND_CALL handler, type
   sub sp, sp, #256
   stp x0,  x1,    [sp, #0]
   stp x2,  x3,    [sp, #16]
   stp x4,  x5,    [sp, #32]
   stp x6,  x7,    [sp, #48]
   stp x8,  x9,    [sp, #64]
   stp x10, x11,   [sp, #80]
   stp x12, x13,   [sp, #96]
   stp x14, x15,   [sp, #112]
   stp x16, x17,   [sp, #128]
   stp x18, x19,   [sp, #144]
   stp x20, x21,   [sp, #160]
   stp x22, x23,   [sp, #176]
   stp x24, x25,   [sp, #192]
   stp x26, x27,   [sp, #208]
   stp x28, x29,   [sp, #224]
   str x30,        [sp, #240]


   mrs x0, esr_el1
   mrs x1, elr_el1
   mov x2, \type
   bl \handler
   
   ldp x0,  x1,  [sp, #0]
    ldp x2,  x3,  [sp, #16]
    ldp x4,  x5,  [sp, #32]
    ldp x6,  x7,  [sp, #48]
    ldp x8,  x9,  [sp, #64]
    ldp x10, x11, [sp, #80]
    ldp x12, x13, [sp, #96]
    ldp x14, x15, [sp, #112]
    ldp x16, x17, [sp, #128]
    ldp x18, x19, [sp, #144]
    ldp x20, x21, [sp, #160]
    ldp x22, x23, [sp, #176]
    ldp x24, x25, [sp, #192]
    ldp x26, x27, [sp, #208]
    ldp x28, x29, [sp, #224]
    ldr x30,      [sp, #240]
    add sp, sp, #256
    eret
.endm
// ---------------------------------------------------
//ValiantCore Aarch64 exceptions.S file vector table
// --------------------------------------------------- 
// Copyright (C) 2026 Bigpower
// ---------------------------------------------------
.align 11
vector_table_el1:

.align 7
    b sync_unhandled
.align 7
    b sync_unhandled
.align 7
    b sync_unhandled
.align 7
    b sync_unhandled

.align 7
curr_el_spx_sync:
   SAVE_AND_CALL sync_handler, #0

.align 7
curr_el_spx_irq:
  SAVE_AND_CALL irq_handler_aarch64, #1

.align 7
curr_el_spx_fiq:
    SAVE_AND_CALL fiq_handler_aarch64, #2

.align 7
curr_el_spx_serror:
    SAVE_AND_CALL serror_handler, #3

// -----When the desktop version named ValiantCore BigPowerOS is released, it will be written using LVGL—meaning it will be a lightweight desktop system.----
.align 7
   b sync_unhandled
.align 7
   b sync_unhandled
.align 7
   b sync_unhandled
.align 7
   b sync_unhandled

// ------ Powered By ValiantCore Aarch64 -----//
.align 7
    b sync_unhandled
.align 7
    b sync_unhandled
.align 7
    b sync_unhandled
.align 7
    b sync_unhandled


// -------exceptions_init is called by C.--------//
sync_unhandled:
  b sync_unhandled

// --------------------------------------------//
exceptions_init:
   ldr x0, =vector_table_el1
   msr vbar_el1, x0
   isb
   ret
// --------------------The End----------------//
