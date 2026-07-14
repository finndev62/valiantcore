# ============================================================
#  ValiantCore Kernel — Makefile
#  Automatic package manager detection + nasm/clang/rustup setup
#  Finds all .c / .asm / .rs files and builds them as X32 / X64
#
#  Copyright (C) 2026 bigpower
#  SPDX-License-Identifier: GPL-2.0-only
# ============================================================

BUILD_DIR   = build
BUILD_CX32  = $(BUILD_DIR)/cX32
BUILD_CX64  = $(BUILD_DIR)/cX64
BUILD_ASMX32 = $(BUILD_DIR)/asmX32
BUILD_ASMX64 = $(BUILD_DIR)/asmX64
BUILD_RUSTX32 = $(BUILD_DIR)/rustX32
BUILD_RUSTX64 = $(BUILD_DIR)/rustX64

# ------------------------------------------------------------
# Auto-discover source files (excluding build/)
# ------------------------------------------------------------
C_SOURCES   := $(shell find . -name "*.c"   -not -path "./$(BUILD_DIR)/*")
C_SOURCES   := $(filter-out %64.c,$(C_SOURCES))
@head -n 1 $< | grep -Fq '/* Finn Dev */' && echo "Skipping tool: $<" || $(CC) -c $< -o $@
ASM_SOURCES := $(shell find . -name "*.asm" -not -path "./$(BUILD_DIR)/*")
RS_SOURCES  := $(shell find . -name "*.rs"  -not -path "./$(BUILD_DIR)/*")

CFLAGS = -ffreestanding -O2 -Iinclude -integrated-as
NASM   = nasm
RUSTC  = rustc
RUSTFLAGS_X32 = --target i686-unknown-none --crate-type staticlib -C opt-level=2
RUSTFLAGS_X64 = --target x86_64-unknown-none --crate-type staticlib -C opt-level=2

# ------------------------------------------------------------
# Auto-detect package manager
# ------------------------------------------------------------
PKG_MANAGER := $(shell \
	if command -v apt-get >/dev/null 2>&1; then echo apt; \
	elif command -v dnf >/dev/null 2>&1; then echo dnf; \
	elif command -v pacman >/dev/null 2>&1; then echo pacman; \
	elif command -v apk >/dev/null 2>&1; then echo apk; \
	elif command -v zypper >/dev/null 2>&1; then echo zypper; \
	else echo none; fi)

.PHONY: all x32 x64 deps check-tools clean help

all: deps x32 x64

# ------------------------------------------------------------
# Check / install dependencies
# ------------------------------------------------------------
deps: check-tools

check-tools:
	@echo "==> Detected package manager: $(PKG_MANAGER)"
	@if command -v sudo >/dev/null 2>&1; then SUDO="sudo"; else SUDO=""; fi; \
	if dpkg -l rustc >/dev/null 2>&1 && [ "$(PKG_MANAGER)" = "apt" ]; then \
		echo "==> Removing conflicting 'rustc' package (rustup will replace it)..."; \
		$$SUDO apt-get remove -y rustc 2>/dev/null || true; \
	fi; \
	missing=0; \
	command -v make   >/dev/null 2>&1 || { echo "  [!] make not found";   missing=1; }; \
	command -v nasm   >/dev/null 2>&1 || { echo "  [!] nasm not found";   missing=1; }; \
	command -v clang  >/dev/null 2>&1 || { echo "  [!] clang not found";  missing=1; }; \
	command -v rustup >/dev/null 2>&1 || { echo "  [!] rustup not found"; missing=1; }; \
	if [ "$$missing" = "1" ]; then \
		echo "==> Installing missing tools ($(PKG_MANAGER))..."; \
		case "$(PKG_MANAGER)" in \
			apt) $$SUDO apt-get update && $$SUDO apt-get install -y build-essential nasm clang rustup ;; \
			dnf) $$SUDO dnf groupinstall -y "Development Tools" && $$SUDO dnf install -y nasm clang rustup ;; \
			pacman) $$SUDO pacman -Sy --noconfirm base-devel nasm clang rustup ;; \
			apk) $$SUDO apk add --no-cache build-base nasm clang rustup ;; \
			zypper) $$SUDO zypper install -y -t pattern devel_basis && $$SUDO zypper install -y nasm clang rustup ;; \
			none) echo "  [ERROR] No supported package manager found, install tools manually."; exit 1 ;; \
		esac; \
	else \
		echo "  [OK] All tools are already installed."; \
	fi
	@echo "==> Configuring rustup default toolchain..."
	@rustup default stable 2>/dev/null || \
		echo "  [!] Could not set default rustup toolchain"
	@echo "==> Adding rustup targets (i686-unknown-none, x86_64-unknown-none)..."
	@rustup target add i686-unknown-none x86_64-unknown-none 2>/dev/null || \
		echo "  [!] Could not add Rust targets, check rustup installation"

# ------------------------------------------------------------
# Create build directories
# ------------------------------------------------------------
$(BUILD_CX32) $(BUILD_CX64) $(BUILD_ASMX32) $(BUILD_ASMX64) $(BUILD_RUSTX32) $(BUILD_RUSTX64):
	mkdir -p $@

# ------------------------------------------------------------
# x32 / x64 targets
# ------------------------------------------------------------
x32: $(BUILD_CX32) $(BUILD_ASMX32) $(BUILD_RUSTX32) \
     $(addprefix $(BUILD_CX32)/, $(notdir $(C_SOURCES:.c=_x32.o))) \
     $(addprefix $(BUILD_ASMX32)/, $(notdir $(ASM_SOURCES:.asm=_x32.o))) \
     $(addprefix $(BUILD_RUSTX32)/, $(notdir $(RS_SOURCES:.rs=_x32.o)))
	@echo "==> x32 build complete -> $(BUILD_CX32) / $(BUILD_ASMX32) / $(BUILD_RUSTX32)"

x64: $(BUILD_CX64) $(BUILD_ASMX64) $(BUILD_RUSTX64) \
     $(addprefix $(BUILD_CX64)/, $(notdir $(C_SOURCES:.c=_x64.o))) \
     $(addprefix $(BUILD_ASMX64)/, $(notdir $(ASM_SOURCES:.asm=_x64.o))) \
     $(addprefix $(BUILD_RUSTX64)/, $(notdir $(RS_SOURCES:.rs=_x64.o)))
	@echo "==> x64 build complete -> $(BUILD_CX64) / $(BUILD_ASMX64) / $(BUILD_RUSTX64)"

# ------------------------------------------------------------
# C files (i386 / x86_64)
# ------------------------------------------------------------
$(BUILD_CX32)/%_x32.o:
	@src=$$(find . -name "$$(basename $@ _x32.o).c" -not -path "./$(BUILD_DIR)/*"); \
	echo "Building C  (32-bit): $$src -> $@"; \
	clang --target=i686-elf -march=i686 -m32 $(CFLAGS) -c $$src -o $@

$(BUILD_CX64)/%_x64.o:
	@src=$$(find . -name "$$(basename $@ _x64.o).c" -not -path "./$(BUILD_DIR)/*"); \
	echo "Building C  (64-bit): $$src -> $@"; \
	clang --target=x86_64-elf -march=x86-64 -m64 $(CFLAGS) -c $$src -o $@

# ------------------------------------------------------------
# Assembly files (i386 / x86_64)
# ------------------------------------------------------------
$(BUILD_ASMX32)/%_x32.o:
	@src=$$(find . -name "$$(basename $@ _x32.o).asm" -not -path "./$(BUILD_DIR)/*"); \
	echo "Assembling  (32-bit): $$src -> $@"; \
	$(NASM) -f elf32 $$src -o $@

$(BUILD_ASMX64)/%_x64.o:
	@src=$$(find . -name "$$(basename $@ _x64.o).asm" -not -path "./$(BUILD_DIR)/*"); \
	echo "Assembling  (64-bit): $$src -> $@"; \
	$(NASM) -f elf64 -d X86_64 $$src -o $@

# ------------------------------------------------------------
# Rust files (i386 / x86_64)
# ------------------------------------------------------------
$(BUILD_RUSTX32)/%_x32.o:
	@src=$$(find . -name "$$(basename $@ _x32.o).rs" -not -path "./$(BUILD_DIR)/*"); \
	echo "Building Rust (32-bit): $$src -> $@"; \
	$(RUSTC) $(RUSTFLAGS_X32) --emit=obj -o $@ $$src

$(BUILD_RUSTX64)/%_x64.o:
	@src=$$(find . -name "$$(basename $@ _x64.o).rs" -not -path "./$(BUILD_DIR)/*"); \
	echo "Building Rust (64-bit): $$src -> $@"; \
	$(RUSTC) $(RUSTFLAGS_X64) --emit=obj -o $@ $$src

# ------------------------------------------------------------
# Clean
# ------------------------------------------------------------
clean:
	rm -rf $(BUILD_DIR)
	@echo "==> build/ directory cleaned."

# ------------------------------------------------------------
# Help
# ------------------------------------------------------------
help:
	@echo "Usage:"
	@echo "  make            - install dependencies + build x32 + x64"
	@echo "  make deps       - check/install nasm, clang, rustup only"
	@echo "  make x32        - build i386 (32-bit) only"
	@echo "  make x64        - build x86_64 (64-bit) only"
	@echo "  make clean      - remove the build/ directory"
