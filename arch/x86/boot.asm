bits 32

section .multiboot2
align 8

mb2_start:
    dd 0xE85250D6
    dd 0
    dd mb2_end - mb2_start
    dd -(0xE85250D6 + 0 + (mb2_end - mb2_start))


    align 8
    dw 5
    dw 0
    dw 20
    dw 1024
    dw 768
    dw 32

    ; End Tag
    align 8
    dw 0
    dw 0
    dd 8
mb2_end:

section .text
global _start
extern kmain


_start:
%ifdef X86_64
   mov esp, stack_space

   mov eax, cr0
   and eax, 0x7FFFFFFF
   mov cr0, eax

   mov eax, cr4
   or eax, 0x20
   mov cr4, eax


   lgdt [gdt64_ptr]

   mov ecx, 0xC0000080
   rdmsr
   or eax, 0x100
   wrmsr

   mov eax, cr0
   or eax, 0x80000001
   mov cr0, eax

   jmp 0x08:long_mode_start
bits 64
long_mode_start:
   mov rsp, stack_space
   mov rdi, [mb2_info_ptr]
   call kmain
   hlt
%else
   mov esp, stack_space
   push ebx
   call kmain
   hlt
%endif

%ifdef X86_64
gdt64:
    dq 0x0000000000000000
    dq 0x00AF9A000000FFFF
    dq 0x00AF92000000FFFF
gdt64_ptr:

  dw $ - gdt64 - 1
  dd gdt64

section .bss   
mb2_info_ptr: resd 1
%endif

section .bss
resb 8192
stack_space:
