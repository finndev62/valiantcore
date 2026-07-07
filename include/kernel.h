#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>

#ifdef __x86_64__
    typedef uint64_t addr_t;
    typedef uint64_t pte_t;
#else
    typedef uint32_t addr_t;
    typedef uint32_t pte_t;
#endif 


#ifdef __x86_64__
   #define SYSCALL_READ   0
   #define SYSCALL_WRITE  1
   #define SYSCALL_OPEN   2
   #define SYSCALL_CLOSE  3
   #define SYSCALL_EXIT   60
#else
   #define SYSCALL_READ  3
   #define SYSCALL_WRITE 4
   #define SYSCALL_OPEN  5
   #define SYSCALL_CLOSE 6
   #define SYSCALL_EXIT  1
#endif

#define SYSCALL_BLOCKED_INT80   0x80
#define SYSCALL_BLOCKED_PTRACE  0x77
#define SYSCALL_BLOCKED_KILL    0x65
#define SYSCALL_BLOCKED_INVALID 0xFF

#ifdef __x86_64__
struct registers {
    addr_t r11, r10, r9, r8, rbp, rdi, rsi, rdx, rcx, rbx, rax;
    addr_t int_no, err_code;
    addr_t rip, cs, rflags, rsp, ss;
};
#else
struct registers {
    addr_t ds;
    addr_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    addr_t int_no, err_code;
    addr_t eip, cs, eflags, useresp, ss;
};
#endif


static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outw(uint16_t port, uint16_t val) {
    asm volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    asm volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outl(uint16_t port, uint32_t val) {
   asm volatile ("outl %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint32_t inl(uint16_t port) {
    uint32_t ret;
    asm volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void kprint(char *message);
void init_gdt();
void init_idt();
void init_scheduler();
int monitor_system_integrity();
void     pic_init(void);
void     pic_send_eoi(uint8_t irq);
void     pic_set_mask(uint8_t irq);
void     pic_clear_mask(uint8_t irq);
void     pic_disable(void);
uint16_t pic_get_irr(void);
uint16_t pic_get_isr(void);

void keyboard_handler(void);
char keyboard_getchar(void);
int  keyboard_has_input(void);
int fat32_init(void);
int fat32_ls(const char *path);
int fat32_open(const char *path, uint8_t write);
int fat32_read(int fd, uint8_t *buf, uint32_t len);
int fat32_close(int fd);

void     pit_init(uint32_t hz);
void     pit_handler(void);
uint32_t pit_get_ticks(void);
uint32_t pit_get_hz(void);
void     pit_sleep(uint32_t ms);
void     kernel_panic(struct registers regs);
int sys_security_audit(uint32_t syscall_id);
int sys_validate_args(uint32_t syscall_id, addr_t addr, uint32_t size);
int sys_read(uint32_t fd, addr_t buf, uint32_t count);
int sys_write(uint32_t fd, addr_t buf, uint32_t count);
void sys_exit(int code);
uint32_t sys_get_count();
uint32_t sys_get_blocked_count();

#endif
