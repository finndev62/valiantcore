/* I'm writing a header file for 64-bit support so that paging.c is
* readable and there's no #ifndef dump.
* What other features should be added to the kernel?
* Multitasking capability coming soon.
* This kernel is free of excess junk.
* I'll write about Bluetooth/WİFİ driver specifications soon, even
* if it's difficult.
*/



#ifndef MMU_H
#define MMU_H

#include <stdint.h>

#ifdef __x86_64__
   typedef uint64_t addr_t;
   typedef uint64_t pte_t;
   #define PAGE_LEVELS 4
   #define ADDR_MASK 0xFFFFFFFFFFFFF000
   #define PT_SHIFT 12
   #define PD_SHIFT 21
   #define PDP_SHIFT 30
   #define PML4_SHIFT 39
#else
   typedef uint32_t addr_t;
   typedef uint32_t pte_t;
   #define PAGE_LEVELS 2
   #define ADDR_MASK 0xFFFFF000
   #define PT_SHIFT 12
   #define PD_SHIFT 22
#endif

#define PAGE_PRESENT 0x1
#define PAGE_RW      0x2
#define PAGE_USER    0x4

#define PT_INDEX(vaddr) (((vaddr) >> 12) & 0x3FF)
#define PD_INDEX(vaddr) (((vaddr) >> 22) & 0x3FF)


extern void* request_physical_page();
extern void security_panic(char* msg, addr_t addr);
extern void arch_mmu_tlb_flush();
#endif
