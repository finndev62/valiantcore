/*
* ValiantCore Kernel
* Copyright (C) 2026 bigpower
* SPDX-License-Identifier: GPL-2.0-only
*/
#include <asm/kernel.h>
#include <stdint.h>

void sync_handler(uint64_t esr, uint64_t elr, uint64_t type) {
     (void)type;
     kernel_panic_aarch64(esr, elr);

}
