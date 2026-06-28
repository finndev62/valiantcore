# Welcome To ValiantCore 

Hello I'm Finn Dev 

I plan to write a lightweight, user-friendly, fast, and secure operating system that supports both x86 and ARM architectures. I am starting by writing my own kernel from scratch—I will not be using any existing kernel. For my target OS, "BigPowerOS," I am building everything—including the kernel—entirely from the ground up. I am working with a zero-dollar budget and developing on a mobile device rather than a computer; if you would like to support me, you can do so by clicking the "Buy Me a Coffee" button.



```bash

# fedora Distros: sudo dnf install git -y
# Debian Distros: sudo apt install git -y
# arch Distros: sudo pacman -S git -y
# Android Termux: pkg install git -y
# Alpine Distro : apk add git -y

# Clone repository
git clone https://github.com/finndev62/valiantcore.git
cd valiantcore

# compile, automatically installs the necessary tools, produces x32 + x64
make

# Only for 32 bit (i386)
make x32

# 64-bit only (x86_64)
make x64

# clean build/ folder
make clean

# List all commands
make help

# for aarch64 compilation
chmod 755 build.sh
./build.sh
``` 
<br>
<a href="https://www.buymeacoffee.com/bigpower21k" target="_blank">
  <img src="https://cdn.buymeacoffee.com/buttons/v2/default-yellow.png" alt="Buy Me A Coffee" style="height: 60px !important;width: 217px !important;" >
</a>
