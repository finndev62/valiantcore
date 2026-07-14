Hello. Here, I will provide details on the executable .vef file format—specifically regarding signature hashes and how to compile files into the .vef format. I haven't written a .vef loader yet, but I plan to do so soon.

for x86_64

``` bash
# Guide to creating a ValiantCore-compatible VEF file with the correct magic signature
# I placed the sample program `welcome.c` in the directory.
# 1. Make code an independent object file with Clang.
clang -target x86_64-none-elf -ffreestanding -nostdlib -O2 -c welcome.c -o app.o

# 2. #2. Convert base address to raw binary by 0x100000 with lld
ld.lld -Ttext 0x100000 --image-base=0x100000 --oformat binary app.o -o app.bin

# 3. Embed the signature with vef-pack
./vef-pack x86_64 app.bin welcome.vef
```

 
 for intel i386
 ``` bash
 
clang -target i386-none-elf -ffreestanding -nostdlib -O2 -c welcome.c -o app.o

ld.lld -Ttext 0x100000 --image-base=0x100000 --oformat binary app.o -o app.bin

./vef-pack i386 app.bin /bin/app_i386.vef
```

for aarch64
``` bash
clang -target aarch64-none-elf -ffreestanding -nostdlib -O2 -c app.c -o app.o
ld.lld -Ttext 0x100000 --image-base=0x100000 --oformat binary app.o -o app.bin
./vef-pack aarch64 app.bin /bin/app_arm64.vef

```
WARNING: These architectures were written based on which architectures Valiant Core supports. If Valiant Core supports other architectures in the future, I will add them here.


