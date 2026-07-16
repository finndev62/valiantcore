/*
* ValiantCore Kernel
* Copyright (C) 2026 bigpower
* SPDX-License-Identifier: GPL-2.0-only
*/
#ifndef VALIANTCORE_STDIO_H
#define VALIANTCORE_STDIO_H
/* A Library for generating stdio.h vef files that
* comply with 2026 ValiantCore system calls.
*/

#include <stddef.h>
#include <stdarg.h>

/* ValiantCore system calls are compatible with the
* supported architectures.
*/


static inline void sys_write(int fd, const char* buf, size_t size) {
   #if defined(__x86_64__)

       register long rdi_reg __asm__("rdi") = fd;
       register const char* rsi_reg __asm__("rsi") = buf;
       register size_t rdx_reg __asm__("rdx") = size;
       register long rax_reg __asm__("rax") = 1;


       __asm__ __volatile__(
            "syscall"
            : "+r"(rax_reg)
            : "r"(rdi_reg), "r"(rsi_reg), "r"(rdx_reg)
            : "rcx", "r11", "memory"
          );

       #elif defined(__i386__)

           __asm__ __volatile__(
               "movl $1, %%eax\n\t"
               "int $0x80"
               :
               : "b"(fd), "c"(buf), "d"(size)
               : "eax", "memory"
             );

       #elif defined(__aarch64__)

           register long x0_reg __asm__("x0") = fd;
           register const char* x1_reg __asm__("x1") = buf;
           register size_t x2_reg __asm__("x2") = size;
           register long x8_reg __asm__("x8") = 1;

           __asm__ __volatile__(
              "svc #0"
              : "+r"(x8_reg)
              : "r"(x0_reg), "r"(x1_reg), "r"(x2_reg)
              : "memory"
            );

       #endif
       }

       static inline size_t sys_read(int fd, char* buf, size_t size) {
          size_t bytes_read = 0;
          #if defined(__x86_64__)
              register long rdi_reg __asm__("rdi") = fd;
              register char* rsi_reg __asm__("rsi") = buf;
              register size_t rdx_reg __asm__("rdx") = size;
              register long rax_reg __asm___("rax")  = 2;
              

              __asm__ __volatile__(
                 "syscall"
                 : "=a"(bytes_read)
                 : "r"(rdi_reg), "r"(rsi_reg), "r"(rdx_reg)
                 : "rcx", "r11", "memory"
          );

      #elif defined(__i386__)
         __asm__ __volatile__(
            "movl $2, %%eax\n\t"
            "int $0x80"
            : "a="(bytes_read)
            : "b"(fd), "c"(buf), "d"(size)
            : "memory"

         );

       #elif defined(__aarch64__)
          register long x0_reg __asm__("x0") = fd;
          register char* x1_reg __asm__("x1") = buf;
          register size_t x2_reg __asm__("x2") = size;
          register long x8_reg  __asm__("x8") = 2;

          __asm__ __volatile__(
            "svc #0"
            : "=r"(bytes_read), "+r"(x8_reg)
            : "r"(x0_reg), "r"(x1_reg), "r"(x2_reg)
            : "memory"
         );
       #endif
       return bytes_read;
    }


    static inline void valiant_itoa(long num, char* str, int base) {
        int i = 0;
        int is_negative = 0;

        if (num == 0) {
           str[i++] = '0';
           str[i]   = '\0';
           return;
         }

         if (num < 0 && base == 10) {
            is_negative = 1;
             num = -num;
          }

          while (num != 0) {
             int rem = num % base;
             str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
             num = num / base;
          }


          if (is_negative) str[i++] = '-';
          str[i] = '\0';

          int start = 0;
          int end = i -1;
          while (start < end) {
              char temp = str[start];
              str[start] = str[end];
              str[end] = temp;
              start++; end--;
           }
       }



static inline int putchar(int character) {
    char c = (char)character;
    sys_write(1, &c, 1);
    return character;

}


static inline int puts(const char* str) {
   if (str == NULL) return -1;
   size_t len = 0;
   while (str[len] != '\0') len++;
   sys_write(1, str, len);
   putchar('\n');
   return 0;
}
/* ValiantCore Compatible Universal Print fC Function */


static inline int printf(const char* format, ...) {
   va_list args;
   va_start(args, format);
   int count = 0;
   char num_buf[32];

   while (*format != '\0') {
       if (*format == '%'&& *(format + 1) != '\0') {
           format++;
           if (*format == 'c') {
              char c = (char)va_arg(args, int);
              putchar(c);
              count++;
            }
            else if (*format == 's') {
               const char* s = va_arg(args, const char*);
               if (s == NULL) s = "(null)";
               while (*s != '\0') {
                   putchar(*s); s++; count++;
               }
          }
            else if (*format == 'd') {
                  long d = va_arg(args, int);
                  valiant_itoa(d, num_buf, 10);
                  char* p = num_buf;
                  while	(*p != '\0') { putchar(*p); p++; count++; }
            }
            else if (*format == 'x') {
               unsigned long x  = va_arg(args, unsigned int);
               valiant_itoa(x, num_buf, 16);
               char* p = num_buf;
               while (*p != '\0') { putchar(*p); p++; count++; }
            }
          } else {
              putchar(*format);
              count++;
           }
           format++;
         }
         va_end(args);
         return count;
      }


      static inline int getchar(void) {
         char c = 0;
         sys_read(0, &c, 1);
         return (int)c;
      }


      static inline char* fgets(char* str, int num, void* stream) {
          if (str == NULL || num <= 0) return NULL;


          int index = 0;
          while (index < (num - 1)) {
              char c = (char)getchar();


              if (c == '\0' || c == '\r' || c == '\n') {
                 if (c == '\n') {
                    str[index++] = c;
                    putchar(c);
                  }
                 break;
                }

                str[index++] = c;
                putchar(c);
              }
              str[index] = '\0';
              return str;
           }
/**************************************************************************
* A quick warning: the `stdio.h` I have written                           *
* here is intended solely for the ValiantCore ecosystem                   *
* When writing code that utilizes this library, make sure                 *
* to use `"stdio.h"` with double quotes rather than `stdio.h'`            *
* with angle brackets would result in using an standart `stdio.h`         *
* that is incompatible with ValiantCore, whereas double quotes ensure the *
* use of the ValiantCore—compatible header.                               *
***************************************************************************/

#endif /* VALIANTCORE_STDIO */

       
          
                  
             
         
        

        
  
