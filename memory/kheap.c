#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <kheap.h>



static uint8_t heap_bitmap[KERNEL_HEAP_SIZE / BLOCK_SIZE / 8];

extern bool get_bitmap(uint32_t bit);

void set_bitmap(uint32_t bit) {
     heap_bitmap[bit / 8] |= (1 << (bit % 8));
}

void clear_bitmap(uint32_t bit) {
     heap_bitmap[bit / 8] &= ~(1 << (bit % 8));
}

void* kmalloc_ext(size_t size) {

     size_t header_size = sizeof(uintptr_t);
     size_t blocks_needed= (size + header_size + BLOCK_SIZE - 1) / BLOCK_SIZE;

     for (size_t i = 0; i < (KERNEL_HEAP_SIZE / BLOCK_SIZE); i++) {
         bool found = true;

        for (size_t j = 0; j < blocks_needed; j++) {
         if (get_bitmap(i + j)) {
            found = false;
                break;
           }
}

if (found) {

   for (size_t j = 0; j < blocks_needed; j++) {
       set_bitmap(i + j);
}

uintptr_t addr = (uintptr_t)(KERNEL_HEAP_START + (i * BLOCK_SIZE));

  *(uintptr_t*)addr = 0xDEADBEEF;

    return (void*)(addr + header_size);
    }
}

return NULL;
}
