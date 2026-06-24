 ;
 ; ValiantCore Kernel
 ; Copyright (C) 2026 bigpower
 ; SPDX-License-Identifier: GPL-2.0-only
 ;

%ifdef X86_64
bits 64
%else
bits 32
%endif

extern irq_handler
extern isr_handler

%macro ISR_NOERRCODE 1
    global isr%1
    isr%1:
        push byte 0
        push byte %1
        jmp isr_common_stub
%endmacro

%macro ISR_ERRCODE 1
    global isr%1
    isr%1:
         push byte %1
         jmp isr_common_stub
%endmacro

ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14


%macro IRQ 2
   global irq%1
   irq%1:
       push byte 9
       push byte %2
       jmp irq_common_stub
%endmacro

IRQ 0, 32
IRQ 1, 33

isr_common_stub:
%ifdef X86_64
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11

    call isr_handler

    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    add rsp, 16
    iretq
%else
    pusha
    mov ax, ds
    push eax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call isr_handler

    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popa
    add esp, 8
    iret
%endif

; -------------------
; ValiantCore Power |
; -------------------

irq_common_stub:
%ifdef X86_64
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11

    call irq_handler

    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    add rsp, 16
    iretq
%else
    pusha
    mov ax, ds
    push eax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call irq_handler

    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popa
    add esp, 8
    iret
%endif
; -------------------------------------------------------
; ValiantCore idt x86 64 incompatibilities fixed.       |
; all ValiantCore x86 64 incompatibilities resolved.
; The system is now working, arm support will come soon.|
; -------------------------------------------------------
global idt_flush
idt_flush:
%ifdef X86_64
   mov rax, [rsp + 8]
   lidt [rax]
   ret
%endif
