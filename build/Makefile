CC = clang
ARCH ?= x86_32

ROOT_DIR = ..
KERN_DIR = $(ROOT_DIR)/kernel
DRV_DIR  = $(ROOT_DIR)/drivers
INC_DIR  = $(ROOT_DIR)/include

ifeq ($(ARCH), x86_64)
    CFLAGS = --target=x86_64-elf -ffreestanding -O2 -Wall -Wextra -I$(INC_DIR) -D__x86_64__
    SUFFIX = x64
else
    CFLAGS = --target=i686-elf -ffreestanding -O2 -Wall -Wextra -I$(INC_DIR) -D__i386__
    SUFFIX = x32
endif

all: ata_$(SUFFIX).o kmain_$(SUFFIX).o

ata_$(SUFFIX).o: $(DRV_DIR)/ata.c
	$(CC) $(CFLAGS) -c $< -o $@

kmain_$(SUFFIX).o: $(KERN_DIR)/kmain.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f ata_*.o kmain_*.o
