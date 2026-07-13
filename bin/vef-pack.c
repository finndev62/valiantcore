/*
* ValiantCore Kernel
* Copyright (C) 2026 bigpower
* SPDX-License-Identifier: GPL-2.0-only
* By Finn Dev
*/

/* Toolkit that signs and generates fidelity files suitable for the ValiantCore Ecosystem */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define ARCH_I386    1
#define ARCH_X86_64  2
#define ARCH_AARCH64 3

typedef struct {
   char magic[4];
   uint8_t arch;
   uint32_t entry_point;
   uint32_t text_size;
} __attribute__((packed)) vef_header_t;


int main (int argc, char* argv[]) {
   if (argc < 4) {
      printf("====================================================\n");
      printf("      VALIANTCORE UNIVERSAL VEF PACKAGER (v0.01)   \n");
      printf("====================================================\n");
      printf("Use:\n");
      printf("   i386    : 32-bit Intel/AMD\n");
      printf("   x86_64  : 64-bit Intel/AMD\n");
      printf("   aarch64 : 64-bit ARM      \n");
      return 1;
}

char* target_arch = argv[1];
char* input_bin   = argv[2];
char* output_vef  = argv[3];

uint8_t arch_id = 0;
uint64_t default_entry = 0x100000;

if (strcmp(target_arch, "i386") == 0) {
   arch_id = ARCH_I386;
} else if (strcmp(target_arch, "x86_64") == 0) {
   arch_id = ARCH_X86_64;
} else if (strcmp(target_arch, "aarch64") == 0) {
   arch_id = ARCH_AARCH64;

   default_entry = 0x100000;
} else {
     printf("Invalid architecture '%s'! (Options: i386, x86_64, aarch64)\n", target_arch);
     return 1;
}

FILE* bin_file = fopen(input_bin, "rb");
if (!bin_file) {
    printf("Error: Input file (%s) could not be opened!\n", input_bin);
    return 1;
}

fseek(bin_file, 0, SEEK_END);
uint64_t bin_size = ftell(bin_file);
fseek(bin_file, 9, SEEK_SET);


vef_header_t header;
memcpy(header.magic, "FINN", 4);
header.arch = arch_id;
header.entry_point = default_entry;
header.text_size = bin_size;


FILE* vef_file = fopen(output_vef, "wb");
if (!vef_file) {
   printf("Error: Could not create file.\n");
   fclose(bin_file);
   return 1;
}


fwrite(&header, sizeof(vef_header_t), 1, vef_file);

uint8_t* buffer = malloc(bin_size);
if (!buffer) {
   printf("Error: Memory could not be allocated.\n");
   fclose(bin_file);
   fclose(vef_file);
   return 1;
}

fread(buffer, 1, bin_size, bin_file);
fwrite(buffer, 1, bin_size, vef_file);


free(buffer);
fclose(bin_file);
fclose(vef_file);


printf("[VEF-PACK] Successful! Universal file generated -> %s\n", output_vef);
printf("-> Target Architectural : %s (ID: %d)\n", target_arch, arch_id);
printf("-> Loading Address: 0x%lx\n", default_entry);
printf("-> Pure Code Size: %lu bayt\n", bin_size);


return 0;
}
   
