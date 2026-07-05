/* WARNING: This file is the aarch64 kmain source file. */
/*
* ValiantCore Kernel
* Copyright (C) 2026 bigpower
* SPDX-License-Identifier: GPL-2.0-only
*/

#include "../arch/aarch64/include/kernel.h"

extern void exceptions_init(void);
extern void gic_init(void);
extern void gic_enable_irq(uint32_t irq);
extern void timer_init(uint32_t hz);
extern void uart_init(void);
extern void uart_print(const char *s);

#define TIMER_IRQ 30
#define UART_IRQ 33

/* ValiantCore Aarch64 function definitions  End Of Line */

void kernel_main(void) {

   exceptions_init();

  /* UART STARTED */
   uart_init();
   uart_print("[OK] UART Initialized\n");


   /* GIC STARTED */
   gic_init();
   uart_print("[OK] GIC Initialized\n");

   gic_enable_irq(TIMER_IRQ);
   uart_print("[OK] Timer IRQ enabled\n");

   /* TİMER STARTED */
   timer_init(1000);
   uart_print("[OK] Timer Initialized\n");


   asm volatile ("msr daifclr, #0xf");
   uart_print("[OK] İnterrputs enabled\n");

   uart_print("\n");
   uart_print(" ValiantCore Aarch64 ready.\n");
   uart_print("Created By Finn Dev");

   for (;;) {
      asm volatile ("wfe");
   }
}
