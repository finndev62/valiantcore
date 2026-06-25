#!/bin/bash

# Configuration
SRC_DIR="arch/aarch64"
BUILD_DIR="$SRC_DIR/build"
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

# Compile .S and .c files
find "$SRC_DIR" -maxdepth 1 -name "*.S" -o -name "*.c" | while read -r file; do
    
    filename=$(basename "$file")
    objname="${filename%.*}.o"
    
    echo "Compiling: $file -> $BUILD_DIR/$objname"
    
    # Run clang with appropriate flags
    clang -c $CLANG_FLAGS "$file" -o "$BUILD_DIR/$objname"
    
    if [ $? -ne 0 ]; then
        echo "Error: Failed to compile $file"
        exit 1
    fi
done

echo "Build process completed successfully. Files are located in: $BUILD_DIR"
