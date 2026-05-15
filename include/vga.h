#ifndef VGA_H
#define VGA_H



/* ----------VGA CONSTANTS--------------------------------- */
#define VGA_BASE      0xB80000
#define VGA_COLS      80
#define VGA_ROWS      25
#define VGA_REG_CTRL  0x3D4
#define VGA_REG_DATA  0x3D5

/* ---------------------- COLORS ------------------------ */
#define VGA_BLACK        0
#define VGA_BLUE         1
#define VGA_GREEN        2
#define VGA_CYAN         3
#define VGA_RED          4
#define VGA_MAGENTA      5
#define VGA_BROWN        6
#define VGA_WHITE        7
#define VGA_BRIGHT_BLUE  9
#define VGA_BRIGHT_CYAN  11
#define VGA_BRIGHT_WHITE 15

void kclr();
void kputc(char c);
void kprint(char *message);
void kprint_color(char *msg, unsigned char fg, unsigned char bg);
void kprint_banner();

#endif
