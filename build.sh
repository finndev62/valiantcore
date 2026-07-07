#!/bin/bash

# Configuration
SRC_DIR="arch/aarch64"
# Main build directory now points to build/aarch64 at the project root
BUILD_DIR="build/aarch64"
TARGET="aarch64-linux-gnu"

mkdir -p "$BUILD_DIR"

# Check system architecture
ARCH=$(uname -m)
echo "Detected architecture: $ARCH"

if [ "$ARCH" != "aarch64" ]; then
    echo "Architecture mismatch. Checking for cross-compilation tools..."
    
    # Identify package manager and install cross-compiler
    if command -v apt-get >/dev/null 2>&1; then
        sudo apt-get update && sudo apt-get install -y gcc-aarch64-linux-gnu clang
    elif command -v dnf >/dev/null 2>&1; then
        sudo dnf install -y gcc-aarch64-linux-gnu clang
    elif command -v pacman >/dev/null 2>&1; then
        sudo pacman -S --noconfirm aarch64-linux-gnu-gcc clang
    elif command -v zypper >/dev/null 2>&1; then
        sudo zypper install -y cross-aarch64-gcc clang
    else
        echo "Error: Could not detect a supported package manager to install cross-compiler."
        exit 1
    fi
    
    # Set compilation flag for cross-compilation
    CLANG_FLAGS="--target=$TARGET"
else
    echo "Architecture is aarch64. Proceeding with native compilation."
    CLANG_FLAGS=""
fi

# ── KERNEL FLAGS AND AUTOMATIC INCLUDE PATHS ────────────────────────────
# -Iarch/aarch64/include allows you to use <asm/kernel.h> directly in C files.
KERNEL_FLAGS="-ffreestanding -O2 -Iinclude -Iarch/aarch64/include"
CLANG_FLAGS="$CLANG_FLAGS $KERNEL_FLAGS"

echo "Using flags: $CLANG_FLAGS"
echo "----------------------------------------"

# ── AUTOMATICALLY FIND AND COMPILE ALL *64.c AND .S FILES ───────────────
# Changed -name "*.c" to -name "*64.c" to strictly filter specific files
find "$SRC_DIR" -type f \( -name "*64.c" -o -name "*.S" \) | while read -r file; do
    
    # Calculate the relative subdirectory path to maintain folder structure
    relative_path="${file#$SRC_DIR/}"
    dir_part=$(dirname "$relative_path")
    filename=$(basename "$file")
    objname="${filename%.*}.o"
    
    # Create matching subdirectories inside the main build/aarch64 folder
    if [ "$dir_part" != "." ]; then
        mkdir -p "$BUILD_DIR/$dir_part"
        target_obj="$BUILD_DIR/$dir_part/$objname"
    else
        target_obj="$BUILD_DIR/$objname"
    fi
    
    echo "Compiling: $file -> $target_obj"
    
    # Run clang with target, freestanding, and include path flags
    clang -c $CLANG_FLAGS "$file" -o "$target_obj"
    
    if [ $? -ne 0 ]; then
        echo "Error: Failed to compile $file"
        exit 1
    fi
done

echo "----------------------------------------"
echo "Build process completed successfully. Files are located in: $BUILD_DIR"
