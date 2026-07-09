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

#endif

