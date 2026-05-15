bits 32


section .multiboot
    align 4
    dd 0x1BADB002
    dd 0x00
    dd -(0x1BADB002 + 0x00)

section .text
global _start
extern kmain

_start
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
   call kmain
   hlt
%else

    mov esp, stack_space
    call kmain
    hlt
%endif

; ----------------------------------------------------------
%ifdef X86_64
gdt64:
   dq 0x000000000000000
   dq 0x00AF9A000000FFF
   dq 0x00AF92000000FFF
gdt64_ptr
   dw $ - gdt64 -1
   dd gdt64
%endif

section .bss
resb 8192
stack_space:
