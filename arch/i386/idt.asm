%ifdef X86_64
bits 64
%else
bits 32
%endif

extern irq_handler
extern exception_handler

%macro ISR_NOERRCODE 1
    global isr%1
    isr%1:
      push byte 0
      push byte %1
      jmp isr_common_stub
%endmacro

ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 8
ISR_NOERRCODE 13
ISR_NOERRCODE 14

%macro IRQ 2
    global irq%1
            push byte 0
            push byte %2
            jmp irq_common_stub
%endmacro

IRQ 0, 32
IRQ 1, 33
IRQ 9, 41

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

    call exception_handler

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

    call exception_handler

    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popa
    add esp, 8
      iret
%endif

; ------------------------IRQ COMMON STUB-------------------
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

     pop ebx
     mov ds, ax
     mov es, ax
     mov fs, ax
     mov gs, ax
     popa
     add esp, 8
     iret
%endif

; -----------IDT FLUSH--------------------------------------
global idt_flush
idt_flush:
%ifdef X86_64
    mov rax, [rsp + 8]
    lidt [rax]
    ret
%else
   mov eax, [esp + 4]
   lidt [eax]
   ret
%endif
