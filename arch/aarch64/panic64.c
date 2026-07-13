/*
* ValiantCore Kernel
* Copyright (C) 2026 bigpower
* SPDX-License-Identifier: GPL-2.0-only
* By Finn Dev
*/
#include <asm/kernel.h>
#include <stdint.h>

static const char *panic_detect_aarch64(uint64_t esr) {
   uint32_t ec = (uint32_t)((esr >> 26) & 0x3F);
/* ValiantCore Aarch64 Panic64 File */
   switch (ec) {
       case 0x00: return "ERR_UNKOWN";
       case 0x01: return "ERR_TRAPPED_WFI_WFE";
       case 0x0E: return "ERR_ILLEGAL_EXEC";
       case 0x15: return "ERR_SVC_AARCH64";
       case 0x20: return "ERR_INST_ABORT_LOWER";
       case 0x21: return "ERR_INST_ABORT";
       case 0x22: return "ERR_PC_ALIGNMENT";
       case 0x24: return "ERR_DATA_ABORT_LOWER";
       case 0x25: return "ERR_DATA_ABORT";
       case 0x26: return "ERR_SP_ALIGNMENT";
       case 0x28: return "ERR_TRAPPED_FPU";
       case 0x2F: return "ERR_SERROR";
       case 0x30: return "ERR_BREAKPOINT_LOWER";
       case 0x31: return "ERR_BREAKPOINT";
       case 0x32: return "ERR_STEP_LOWER";
       case 0x33: return "ERR_STEP";
       case 0x34: return "ERR_WATCHPOINT_LOWER";
       case 0x35: return "ERR_WATCHPOINT";
       case 0x3C: return "ERR_BRK_AARCH64";
       default:   return "ERR_UNKOWN";
     }
}


static void uint64_to_hex(uint64_t val, char *out) {
   const char hex[] = "0123456789ABCDEF";
   out[0] = '0'; out[1] = 'x';
   for (int i = 0; i < 16; i++) 
       out[2 + i] = hex[(val >> (60 - i * 4)) & 0xF];
   out[18] = '\0';
}

void kernel_panic_aarch64(uint64_t esr, uint64_t elr) {
   asm volatile ("msr daifset, #0xf");


   const char *code = panic_detect_aarch64(esr);


   char esr_str[20];
   char elr_str[20];
   uint64_to_hex(esr, esr_str);
   uint64_to_hex(elr, elr_str);


   /* ValiantCore: Designing the Panic State with the uart_print Function */

   uart_print("\r\n");
   uart_print("################################################################################\r\n");
   uart_print("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
   uart_print("!!                                                                            !!\r\n");
   uart_print("!!           ValiantCore AArch64 - A Problem Has Been Detected               !!\r\n");
   uart_print("!!              Please reboot your device to recover.                        !!\r\n");
   uart_print("!!                                                                            !!\r\n");
   uart_print("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
   uart_print("################################################################################\r\n");
   uart_print("\r\n");
   uart_print("                           *** STOP ERROR ***\r\n");
   uart_print("\r\n");
   uart_print("    Error Code  :  ");
   uart_print(code);
   uart_print("\r\n");
   uart_print("    ESR_EL1     :  ");
   uart_print(esr_str);
   uart_print("\r\n");
   uart_print("    ESR_EL1     :  ");
   uart_print(elr_str);
   uart_print("\r\n");
   uart_print("    Architecture:  AArch64\r\n");
   uart_print("\r\n");
   uart_print("################################################################################\r\n");
   uart_print("##                                                                            ##\r\n");
   uart_print("##   If you cannot resolve this issue, please open an issue at:              ##\r\n");
   uart_print("##                                                                            ##\r\n");
   uart_print("##          >>> github.com/finndev62/valiantcore -> Issues tab <<<           ##\r\n");
   uart_print("##                                                                            ##\r\n");
   uart_print("################################################################################\r\n");
   uart_print("\r\n");

  for (;;) asm volatile ("wfe");
}


