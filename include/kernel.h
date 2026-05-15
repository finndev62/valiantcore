#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>

// --- Mimari Bazlı Tip Tanımları ---
#ifdef __x86_64__
    typedef uint64_t addr_t;
    typedef uint64_t pte_t;
#else
    typedef uint32_t addr_t;
    typedef uint32_t pte_t;
#endif // <--- BURASI EKSİKTİ

// --- Syscall Numaraları ---
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

struct registers {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
};

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

int sys_security_audit(uint32_t syscall_id);
int sys_validate_args(uint32_t syscall_id, addr_t addr, uint32_t size);
int sys_read(uint32_t fd, addr_t buf, uint32_t count);
int sys_write(uint32_t fd, addr_t buf, uint32_t count);
void sys_exit(int code);
uint32_t sys_get_count();
uint32_t sys_get_blocked_count();

#endif 
