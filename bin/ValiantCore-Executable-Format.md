Hello. Here, I will provide details on the executable .vef file format—specifically regarding signature hashes and how to compile files into the .vef format. I haven't written a .vef loader yet, but I plan to do so soon.

``` bash
# Guide to creating a ValiantCore-compatible VEF file with the correct magic signature
# I placed the sample program `app.c` in the directory.
# 1. Make code an independent object file with Clang.
clang -target x86_64-none-elf -ffreestanding -nostdlib -O2 -c app.c -o app.o

# 2. #2. Convert base address to raw binary by 0x100000 with lld
ld.lld -Ttext 0x100000 --image-base=0x100000 --oformat binary app.o -o app.bin

# 3. Embed the signature with vef-pack
./vef-pack app.bin welcome.vef

# Vef file can now be run for the ready-made file ValiantCore
