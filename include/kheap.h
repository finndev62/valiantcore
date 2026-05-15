#ifndef KHEAP_H
#define KHEAP_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __x86_64__
    typedef uint64_t addr_t;
    #define KERNEL_HEAP_START 0xFFFF800000000000
#else
    typedef uint32_t addr_t;
    #define KERNEL_HEAP_START 0xD0000000
#endif

#define KERNEL_HEAP_SIZE  0x2000000
#define BLOCK_SIZE        32

void* kmalloc_ext(size_t size);
void kfree(void* ptr);

extern bool get_bitmap(uint32_t bit);
void set_bitmap(uint32_t bit);
void clear_bitmap(uint32_t bit);

#endif
