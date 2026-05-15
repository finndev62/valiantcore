# Derleyici ayarları
CC = gcc
ARCH ?= x86_32

CFLAGS = -ffreestanding -O2 -Wall -Wextra -I./include

ifeq ($(ARCH), x86_64)
    CFLAGS += -m64 -D__x86_64__
    TARGET = titan_kernel_x64
else
    CFLAGS += -m32 -D__i386__
    TARGET = titan_kernel_x32
endif

OBJS = memory/paging.o \
       memory/kheap.o \
       kernel/kernel.o \
       security/security.o

all: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	ld -m $(if $(filter x86_64,$(ARCH)),elf_x86_64,elf_i386) $(OBJS) -o $(TARGET)

clean:
	rm -f memory/*.o kernel/*.o security/*.o $(TARGET)
