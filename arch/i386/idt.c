#include "kernel.h"
#include <stdint.h>



extern void isr0();
extern void isr13();
extern void isr14();
extern void irq0();
extern void irq1();
extern void idt_flush(uint32_t);

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

struct idt_ptr idtp;
struct idt_entry idt[256];


void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_lo = (base & 0xFFFF);
    idt[num].base_hi = (base >> 16) & 0xFFFF;
    idt[num].sel     = sel;
    idt[num].always0 = 0;
    idt[num].flags   = flags;
}

void init_gdt() {
	idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
	idtp.base  = (uint32_t)&idt;


	for(int i = 0; i < 256; i++) idt_set_gate(i, 0, 0, 0);

	idt_set_gate(0,   (uint32_t)isr0, 0x08, 0x8E);
	idt_set_gate(13,  (uint32_t)isr13, 0x08, 0x8E);
	idt_set_gate(14,  (uint32_t)isr14, 0x08, 0x8E);
     
        idt_set_gate(32,  (uint32_t)irq0, 0x08, 0x8E);
	idt_set_gate(33,  (uint32_t)irq1, 0x08, 0x8E);

	idt_flush((uint32_t)&idtp);
    }

    void irq_handler(struct registers regs) {
	 if (regs.int_no == 33) {
           kprint("Keyboard Pressed!\n");
	}
 
	if (regs.int_no >= 40) outb(0xA0, 0x20);
	outb(0x20, 0x20);
}
