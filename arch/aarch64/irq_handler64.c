/*
* ValiantCore Kernel
* Copyright (C) 2026 bigpower
* SPDX-License-Identifier: GPL-2.0-only
*/

#include "include/kernel.h"
#include <stdint.h>

extern uint32_t gic_acknowledge(void);
extern void     gic_end_of_interrupt(uint32_t irq);
extern void     timer_handler(void);

#define TIMER_IRQ 30
#define UART_IRQ  33
#define SPURIOUS 1023

/* ValiantCore Aarch64 definitions end of line */
void irq_handler_aarch64(uint64_t esr, uint64_t elr, uint64_t type) {
    (void)esr;
    (void)elr;
    (void)type;

    uint32_t irq = gic_acknowledge();

    if (irq == SPURIOUS) {

         return;
    }

    if (irq == TIMER_IRQ) {
        timer_handler();
     } else if (irq == UART_IRQ) {

         uart_print("[IRQ] Uart interrput\n");
     } else {
         uart_print("[IRQ] Unknown\n");
     }


     gic_end_of_interrupt(irq);
}

/*==============≠===============================*/
void fiq_handler_aarch64(uint64_t esr, uint64_t elr, uint64_t type) {
     (void)esr;
     (void)elr;
     (void)type;
     uart_print("[FIQ] Fast intterput received\n");
 }
