/*
 * ValiantCore Kernel
 * Copyright (C) 2026 bigpower
 * SPDX-License-Identifier: GPL-2.0-only
 */
-------------------------------------------------------------
 > Developer   : Finn Dev
 > Maintainer  : Finn Dev
------------------------------------------------------------*/


#include "../include/kernel.h"

static const uint32_t blocked_syscalls[] = {
    SYSCALL_BLOCKED_INT80,
    SYSCALL_BLOCKED_PTRACE,
    SYSCALL_BLOCKED_KILL,
    SYSCALL_BLOCKED_INVALID,
};

#define BLOCKED_COUNT (sizeof(blocked_syscalls) / sizeof(blocked_syscalls[0]))

static uint32_t syscall_count = 0;
static uint32_t blocked_count = 0;


extern int security_check_memory(addr_t addr, uint32_t size);
extern int hw_security_chip_alert(uint32_t event, addr_t addr, uint32_t level);

int sys_security_audit(uint32_t syscall_id) {
    syscall_count++;
    for (addr_t i = 0; i < BLOCKED_COUNT; i++) {
        if (syscall_id == blocked_syscalls[i]) {
           blocked_count++;
           kprint("SYSCALL BLOCKED: Dangerous syscall detected!\n");
           hw_security_chip_alert(syscall_id, 0, 3);
           return -1;
        }
    }
    return 0;
 }

 int sys_validate_args(uint32_t syscall_id, addr_t addr, uint32_t size) {
    if  (sys_security_audit(syscall_id) != 0)
         return -1;
    if (addr == 0 || size == 0) {
       kprint("SYSCALL BLOCKED: Null or zero-size arg!\n");
       return -1;
    }
    if (security_check_memory(addr, size) != 0) {
       kprint("SYSCALL BLOCKED: Memory validation failed!\n");
       return -1;
    }
    return 0;
 }

int sys_write(uint32_t fd, addr_t buf, uint32_t count) {
    if (sys_validate_args(SYSCALL_WRITE, buf, count) != 0)
       return -1;
    if (fd > 1024) {
       kprint("[SYSCALL] BLOCKED: Invalid file descriptor!\n");
       return -1;
    }
    
     if (count > 0x100000) {
       kprint("[SYSCALL] BLOCKED: Write size too large!\n");
       return -1;
    }
    return 0;
}
void sys_exit(int code) {
     kprint("[SYSCALLS] INFO Process exit requested.\n");
     while(1);
}


uint32_t sys_get_count()          { return syscall_count; }
uint32_t sys_get_blocked_count()  { return blocked_count; }
    

