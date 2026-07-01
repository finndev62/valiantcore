#include <stdint.h>
#include "../include/io.h"

#define KBD_DATA_PORT   0x60
#define KBD_BUFFER_SIZE 256

#define KBD_RELEASE_MASK 0x80

static const char kbd_map[128] = {
    0,   27,  '1', '2', '3', '4', '5', '6', '7', '8',  /* 0-9   */
    '9', '0', '-', '=', '\b','\t','q', 'w', 'e', 'r',  /* 10-19 */
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,    /* 20-29 (29=ctrl) */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',  /* 30-39 */
    '\'','`', 0,  '\\','z', 'x', 'c', 'v', 'b', 'n',    /* 40-49 (42=lshift) */
    'm', ',', '.', '/', 0,  '*', 0,   ' ', 0,   0,      /* 50-59 (54=rshift,56=alt,57=space) */
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    /* 60-69 (F1-F10) */
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    /* 70-79 */
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0
};

static const char kbd_map_shift[128] = {
    0,   27,  '!', '@', '#', '$', '%', '^', '&', '*',
    '(', ')', '_', '+', '\b','\t','Q', 'W', 'E', 'R',
    'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
    '"', '~', 0,  '|', 'Z', 'X', 'C', 'V', 'B', 'N',
    'M', '<', '>', '?', 0,  '*', 0,   ' ', 0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0
};

static int shift_pressed = 0;
static int ctrl_pressed  = 0;
static int caps_lock     = 0;

 static char kbd_buffer[KBD_BUFFER_SIZE]; 
static uint32_t kbd_head = 0; static uint32_t 
kbd_tail = 0;

static void kbd_buffer_push(char c) {
    uint32_t next = (kbd_head + 1) % KBD_BUFFER_SIZE;
    if (next == kbd_tail) return; /* tampon dolu, karakter kaybolur */
    kbd_buffer[kbd_head] = c;
    kbd_head = next;
}

char keyboard_getchar(void) {
    if (kbd_head == kbd_tail) return 0;
    char c = kbd_buffer[kbd_tail];
    kbd_tail = (kbd_tail + 1) % KBD_BUFFER_SIZE;
    return c;
}

int keyboard_has_input(void) {
    return kbd_head != kbd_tail;
}

 void keyboard_handler(void) {
 uint8_t scancode = inb(KBD_DATA_PORT);

    int released = scancode & KBD_RELEASE_MASK;
    uint8_t code  = scancode & ~KBD_RELEASE_MASK;

    if (code == 42 || code == 54) {
        shift_pressed = !released;
        return;
    }

    if (code == 29) {
        ctrl_pressed = !released;
        return;
    }

    if (code == 58 && !released) {
        caps_lock = !caps_lock;
        return;
    }

    if (released) return;

    if (code >= 128) return; 

    char c = shift_pressed ? kbd_map_shift[code] : kbd_map[code];

    
    if (caps_lock && c >= 'a' && c <= 'z') c -= 32;
    else if (caps_lock && c >= 'A' && c <= 'Z') c += 32;

    if (c != 0) kbd_buffer_push(c);
}
/* --------------------- The End ----------------*/
/* By Finn Dev */
