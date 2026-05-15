bits 32
global _start

extern kmain_64

section .text
_start:

    mov esp, stack_top

    call setup_page_tables
    call enable_paging

    lgdt  [gdt64.pointer]

    jmp 0x08:init_64

bits 64
init_64:

   mov ax, 0
   mov ss, ax
   mov ds, ax
   mov es, ax

   call kmain_64
   hlt
section .bss
align 4096
p4_table: resb 4096
stack_bottom: resb 16384
stack_top:
