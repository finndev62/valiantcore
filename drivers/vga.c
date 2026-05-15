#include <vga.h>
#include <kernel.h>

static unsigned short *vga_buf = (unsigned short *)VGA_BASE;
static int cursor_x = 0;
static int cursor_y = 0;
static unsigned char current_color = 0x0F;

static inline unsigned char vga_color(unsigned char fg, unsigned char bg) {
    return fg | (bg << 4);
}

static void update_cursor() {
    unsigned short pos = cursor_y * VGA_COLS + cursor_x;
    outb(VGA_REG_CTRL, 0x0F);
    outb(VGA_REG_DATA, (unsigned char)(pos & 0xFF));
    outb(VGA_REG_CTRL, 0x0E);
    outb(VGA_REG_DATA, (unsigned char)((pos >> 8) & 0xFF));
}

static void scroll() {
    unsigned short blank = ' ' | (current_color << 8);
    if (cursor_y >= VGA_ROWS) {
        for (int i = 0; i < (VGA_ROWS - 1) * VGA_COLS; i++)
            vga_buf[i] = vga_buf[i + VGA_COLS];
        for (int i = (VGA_ROWS - 1) * VGA_COLS; i < VGA_ROWS * VGA_COLS; i++)
            vga_buf[i] = blank;
        cursor_y = VGA_ROWS - 1;
    }
}

void kclr() {
    unsigned short blank = ' ' | (current_color << 8);
    for (int i = 0; i < VGA_ROWS * VGA_COLS; i++)
        vga_buf[i] = blank;
    cursor_x = 0;
    cursor_y = 0;
    update_cursor();
}

void kputc(char c) {
    if      (c == '\n') { cursor_x = 0; cursor_y++; }
    else if (c == '\r') { cursor_x = 0; }
    else if (c == '\t') { cursor_x = (cursor_x + 4) & ~3; }
    else if (c == '\b') { if (cursor_x > 0) cursor_x--; }
    else {
        vga_buf[cursor_y * VGA_COLS + cursor_x] =
            (unsigned short)c | ((unsigned short)current_color << 8);
        cursor_x++;
    }
    if (cursor_x >= VGA_COLS) { cursor_x = 0; cursor_y++; }
    scroll();
    update_cursor();
}

void kprint_color(char *msg, unsigned char fg, unsigned char bg) {
    unsigned char old = current_color;
    current_color = vga_color(fg, bg);
    while (*msg) kputc(*msg++);
    current_color = old;
}

void kprint(char *message) {
    while (*message) kputc(*message++);
}

void kprint_banner() {
    kclr();
    kprint_color("  CyberArmor OS - BIGPOWERKERNEL  \n", VGA_BRIGHT_WHITE, VGA_BLUE);
    kprint_color("  Arch: ", VGA_BRIGHT_CYAN, VGA_BLUE);
    kprint_color(sizeof(addr_t) == 8 ? "x86_64  \n" : "i386    \n", VGA_BRIGHT_CYAN, VGA_BLUE);
    kprint_color("  Kernel!\n", VGA_GREEN, VGA_BLACK);
}
