/*
 * ValiantCore Kernel
 * Copyright (C) 2026 bigpower
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include "../../include/kernel.h"
#include <stdint.h>

#define PANIC_VGA_BASE  0xB8000
#define PANIC_COLS      80
#define PANIC_ROWS      25

#define PANIC_RED       0x04
#define PANIC_YELLOW    0x4E
#define PANIC_WHITE     0x4F

static volatile uint16_t *panic_vga = (volatile uint16_t *)PANIC_VGA_BASE;

static void panic_putc(int row, int col, char c, uint8_t color) {
    if (row >= PANIC_ROWS || col >= PANIC_COLS) return;
    panic_vga[row * PANIC_COLS + col] = (uint16_t)c | ((uint16_t)color << 8);
}

static void panic_puts(int row, int col, const char *s, uint8_t color) {
    while (*s && col < PANIC_COLS)
        panic_putc(row, col++, *s++, color);
}

static void panic_fill_row(int row, uint8_t color) {
    for (int col = 0; col < PANIC_COLS; col++)
        panic_putc(row, col, ' ', color);
}

static void panic_clear(void) {
    for (int row = 0; row < PANIC_ROWS; row++)
        panic_fill_row(row, PANIC_RED);
}

static void panic_center(int row, const char *s, uint8_t color) {
    int len = 0;
    while (s[len]) len++;
    int col = (PANIC_COLS - len) / 2;
    if (col < 0) col = 0;
    panic_puts(row, col, s, color);
}

static const char *panic_detect(uint32_t int_no) {
    switch (int_no) {
        case 0:  return "ERR_DIVIDE_ZERO";
        case 2:  return "ERR_NMI";
        case 6:  return "ERR_INVALID_OPCODE";
        case 8:  return "ERR_DOUBLE_FAULT";
        case 13: return "ERR_GPF";
        case 14: return "ERR_PAGEFAULT";
        case 32: return "ERR_INTERRUPTS";
        case 33: return "ERR_KEYBOARD";
        default: return "ERR_UNKNOWN";
    }
}

void kernel_panic(struct registers regs) {
    asm volatile ("cli");

    const char *code = panic_detect(regs.int_no);

    panic_clear();

    panic_fill_row(2, PANIC_YELLOW);
    panic_center(2, "!!! VALIANTCORE PANIC !!!", PANIC_YELLOW);

    panic_center(6, "A critical error has been detected in the system.", PANIC_WHITE);

    panic_puts(9,  4, "Error Code : ", PANIC_WHITE);
    panic_puts(9, 16, code,           PANIC_YELLOW);

#ifdef __x86_64__
    panic_puts(11, 4, "Architectural    : x86_64", PANIC_WHITE);
#else
    panic_puts(11, 4, "Architectural    : i386",   PANIC_WHITE);
#endif

    panic_center(19, "Can't you find the cause of the error?", PANIC_WHITE);
    panic_center(20, "For help:", PANIC_WHITE);
    panic_center(21, "github.com/finndev62/valiantcore", PANIC_YELLOW);
    panic_center(22, "-> Use the Issues tab <-", PANIC_WHITE);

    panic_fill_row(24, PANIC_YELLOW);
    panic_center(24, "ValiantCore Kernel — github.com/finndev62", PANIC_YELLOW);

    for (;;) asm volatile ("hlt");
}
