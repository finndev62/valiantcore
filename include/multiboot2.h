#ifndef MULTIBOOT2_H
#define MULTIBOOT2_H

#include <stdint.h>

#define MULTIBOOT2_MAGIC    0x36D76289

#define MB2_TAG_END         0
#define MB2_TAG_CMDLINE     1
#define MB2_TAG_BOOTLOADER  2
#define MB2_TAG_MODULE      3
#define MB2_TAG_MEMINFO     4
#define MB2_TAG_BOOTDEV     5
#define MB2_TAG_MMAP        6
#define MB2_TAG_FRAMEBUFFER 8
#define MB2_TAG_EFI64       12



typedef struct __attribute__ ((packed)) {
    uint32_t type;
    uint32_t size;
} mb2_tag_t;


typedef struct __attribute__ ((packed)) {
    uint32_t type;
    uint32_t size;
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t  framebuffer_bpp;
    uint8_t  framebuffer_type;
    uint16_t reserved;
} mb2_tag_framebuffer_t;



typedef struct __attribute__ ((packed)) {
     uint32_t type;
     uint32_t size;
     uint32_t entry_size;
     uint32_t entry_version;
} mb2_tag_mmap_t;


static inline mb2_tag_t *mb2_find_tag(void *mb2_info, uint32_t type) {

      uint8_t *ptr = (uint8_t *)mb2_info + 8;

      while (1) {
          mb2_tag_t *tag = (mb2_tag_t *)ptr;

          if (tag->type == MB2_TAG_END) return 0;
          if (tag->type == type)        return tag;


          ptr += (tag->size + 7) & ~7;
       }

   }

   #endif /* MULTIBOOT2_H */
   
        

    
