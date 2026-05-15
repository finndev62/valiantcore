#include <kernel.h>

void kmain() {
    init_gdt();
    init_idt();
    init_scheduler();
    monitor_system_integrity();

    kprint("CyberArmor OS Loading...\n");

    while (1);
}
