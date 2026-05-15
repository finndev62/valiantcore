#include <stdint.h>
#include <stdbool.h>
#include <paging.h>

pte_t kernel_page_directory[1024] __attribute__((aligned(4096)));

void map_page(addr_t phys, addr_t virt, uint32_t flags) {

     uint32_t pd_idx = PD_INDEX(virt);
     uint32_t pt_idx = PT_INDEX(virt);

     if (!(kernel_page_directory[pd_idx] & PAGE_PRESENT)) {

     addr_t new_pt = (addr_t)request_physical_page();


    kernel_page_directory[pd_idx] = new_pt | flags | PAGE_PRESENT;
}


pte_t* table = (pte_t*)((addr_t)kernel_page_directory[pd_idx] & ADDR_MASK);
table[pd_idx] = (phys & ADDR_MASK) | flags | PAGE_PRESENT;

arch_mmu_tlb_flush();
}

void page_fault_handler(addr_t faulting_address, uint32_t error_code) {

    if (error_code & 0x1) {
       security_panic("Cyber Armor: Unauthorized writing attempt", faulting_address);
    } else {
       security_panic("Cyber Armor: Invalid memory acces", faulting_address);
    }
}
