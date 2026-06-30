#include "../../include/kernel.h"
#include <stdint.h>

extern void isr0();
extern void isr13();
extern void isr14();
extern void idt_flush(uintptr_t);
extern void isr0();
extern void isr1();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void irq0();
extern void irq1();

#ifdef __x86_64__
struct idt_entry {
    uint16_t base_lo;
    uint16_t sel;
    uint8_t ist;
    uint8_t flags;
    uint16_t base_mid;
    uint32_t base_hi;
    uint32_t always0;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));
#else
struct idt_entry {
    uint16_t base_lo;
    uint16_t sel;
    uint8_t always0;
    uint8_t flags;
    uint16_t base_hi;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));
#endif

struct idt_ptr idtp;
struct idt_entry idt[256];

void idt_set_gate(uint8_t num, uintptr_t base, uint16_t sel, uint8_t flags) {
#ifdef __x86_64__
    idt[num].base_lo = (base & 0xFFFF);
    idt[num].base_mid = (base >> 16) & 0xFFFF;
    idt[num].base_hi = (base >> 32) & 0xFFFFFFFF;
    idt[num].sel     = sel;
    idt[num].ist     = 0;
    idt[num].flags   = flags;
    idt[num].always0 = 0;
#else
    idt[num].base_lo = (base & 0xFFFF);
    idt[num].base_hi = (base >> 16) & 0xFFFF;
    idt[num].sel     = sel;
    idt[num].always0 = 0;
    idt[num].flags   = flags;
#endif
}

void init_idt() {
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base  = (uintptr_t)&idt;

    for(int i = 0; i < 256; i++) idt_set_gate(i, 0, 0, 0);

    idt_set_gate(0,   (uintptr_t)isr0, 0x08, 0x8E);
    idt_set_gate(13,  (uintptr_t)isr13, 0x08, 0x8E);
    idt_set_gate(14,  (uintptr_t)isr14, 0x08, 0x8E);
    idt_set_gate(32,  (uintptr_t)irq0, 0x08, 0x8E);
    idt_set_gate(33,  (uintptr_t)irq1, 0x08, 0x8E);
    idt_set_gate(1,  (uintptr_t)isr1,  0x08, 0x8E);
    idt_set_gate(3,  (uintptr_t)isr3,  0x08, 0x8E);
    idt_set_gate(4,  (uintptr_t)isr4,  0x08, 0x8E);
    idt_set_gate(5,  (uintptr_t)isr5,  0x08, 0x8E);
    idt_set_gate(6,  (uintptr_t)isr6,  0x08, 0x8E);
    idt_set_gate(7,  (uintptr_t)isr7,  0x08, 0x8E);
    idt_set_gate(8,  (uintptr_t)isr8,  0x08, 0x8E);
    idt_set_gate(9,  (uintptr_t)isr9,  0x08, 0x8E);
    idt_set_gate(10, (uintptr_t)isr10, 0x08, 0x8E);
    idt_set_gate(11, (uintptr_t)isr11, 0x08, 0x8E);
    idt_set_gate(12, (uintptr_t)isr12, 0x08, 0x8E);    
    idt_flush((uintptr_t)&idtp);
}

void irq_handler(struct registers regs) {
    if (regs.int_no == 32)
        pit_handler();
    if (regs.int_no == 33)
        keyboard_handler();
    pic_send_eoi(regs.int_no - 32);
}

void isr_handler(struct registers regs) {
    kernel_panic(regs);
}
