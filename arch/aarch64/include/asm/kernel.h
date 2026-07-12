#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>

typedef uint64_t addr_t;


/*  UART Serial Display port function definitions */
void uart_init();
void uart_putc(char c);
void uart_print(const char *s);
char uart_getc(void);
int  uart_has_input(void);
void kernel_panic_aarch64(uint64_t esr, uint64_t elr);
#endif

