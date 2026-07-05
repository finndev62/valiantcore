#include "../../arch/aarch64/include/kernel.h"
#include <stdint.h>


/********************************************** 
* ValiantCore Aarch64 UART File UART Definitions         
***********************************************/

#define UART0_BASE   0x09000000UL
#define UART0_DR     (UART0_BASE + 0x00)
#define UART0_FR     (UART0_BASE + 0x18)
#define UART0_IBRD   (UART0_BASE + 0x24)
#define UART0_FBRD   (UART0_BASE + 0x28)
#define UART0_LCR    (UART0_BASE + 0x2C)
#define UART0_CR     (UART0_BASE + 0x30)
#define UART0_IMSC   (UART0_BASE + 0x38)

#define UART_FR_TXFF (1 << 5)
#define UART_FR_RXFE (1 << 4)

/* ValiantCore UART definitions end of line */
static inline void mmio_write32_uart(addr_t addr, uint32_t val) {
    *(volatile uint32_t *)addr = val;
}

static inline uint32_t mmio_read32_uart(addr_t addr) {
  return *(volatile uint32_t *)addr;
}

/*---------ValiantCore https://finndev62.github.io----*/
void uart_init(void) {

    mmio_write32_uart(UART0_CR, 0x00);

    mmio_write32_uart(UART0_IBRD, 13);
    mmio_write32_uart(UART0_FBRD, 1);

    mmio_write32_uart(UART0_LCR, (3 << 5) | (1 << 4));

    mmio_write32_uart(UART0_IMSC, 0x00);

    mmio_write32_uart(UART0_CR, (1 << 0) | (1 << 8) | (1 << 9));
}


void uart_putc(char c) {
    while (mmio_read32_uart(UART0_FR) & UART_FR_TXFF) {
        asm volatile ("nop");
    }
    mmio_write32_uart(UART0_DR, (uint32_t)c);
}

/*==========================================*/
void uart_print(const char *s) {
    while (*s) {
        if (*s == '\n') {
            uart_putc('\r');
        }
        uart_putc(*s);
        s++;
    }
}

/*==========================================*/
char uart_getc(void) {
     while (mmio_read32_uart(UART0_FR) & UART_FR_RXFE) {
         asm volatile ("nop");
     }
     return (char)(mmio_read32_uart(UART0_DR) & 0xFF);
}
/*==========================================*/
int uart_has_input(void) {
    return !(mmio_read32_uart(UART0_FR) & UART_FR_RXFE);
}
/* -------- The End of Line ------- */
