# ============================================
# ACE Micro-Kernel Makefile (x86 / RISC-V, 32/64-bit)
# ============================================
ARCH ?= x86
BITS ?= 64
UNAME_S := $(shell uname -s)
# --------------------------------------------
# Toolchain selection
# --------------------------------------------
ifeq ($(ARCH),x86)
  ifeq ($(BITS),64)
    CROSS_COMPILE = x86_64-elf-
    QEMU = qemu-system-x86_64
    CFLAGS_ARCH = -m64
    LDFLAGS_ARCH = -m elf_x86_64
  else
    CROSS_COMPILE = i686-elf-
    QEMU = qemu-system-i386
    CFLAGS_ARCH = -m32
    LDFLAGS_ARCH = -m elf_i386
  endif
else ifeq ($(ARCH),riscv)
  ifeq ($(BITS),64)
    CROSS_COMPILE = riscv64-unknown-elf-
    QEMU = qemu-system-riscv64
  else
    CROSS_COMPILE = riscv32-unknown-elf-
    QEMU = qemu-system-riscv32
  endif
  CFLAGS_ARCH =
  LDFLAGS_ARCH =
else
  $(error ❌ Unsupported ARCH. Use ARCH=x86 or ARCH=riscv)
endif
# --------------------------------------------
# Toolchain commands
# --------------------------------------------
CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)as
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
# --------------------------------------------
# Flags
# --------------------------------------------
CFLAGS = $(CFLAGS_ARCH) -ffreestanding -fno-builtin -fno-stack-protector \
         -Wall -Wextra -Iinclude -nostdlib -nostdinc
ASFLAGS =
LDFLAGS = $(LDFLAGS_ARCH)
# --------------------------------------------
# Directories
# --------------------------------------------
BUILD_DIR = build
BOOT_DIR = boot
KERNEL_DIR = kernel
DRIVERS_DIR = drivers
MM_DIR = mm
FS_DIR = fs
LIB_DIR = lib
VGA_DIR = $(DRIVERS_DIR)/vga
# --------------------------------------------
# Sources
# --------------------------------------------
BOOT_SOURCES   = $(wildcard $(BOOT_DIR)/*.s)
KERNEL_SOURCES = $(wildcard $(KERNEL_DIR)/*.c)
DRIVER_SOURCES = $(wildcard $(DRIVERS_DIR)/*.c)
VGA_SOURCES    = $(wildcard $(VGA_DIR)/*.c)
MM_SOURCES     = $(wildcard $(MM_DIR)/*.c)
FS_SOURCES     = $(wildcard $(FS_DIR)/*.c)
LIB_SOURCES    = $(wildcard $(LIB_DIR)/*.c)
# --------------------------------------------
# Objects
# --------------------------------------------
KERNEL_OBJECTS = $(patsubst $(KERNEL_DIR)/%.c, $(BUILD_DIR)/kernel/%.o, $(KERNEL_SOURCES))
DRIVER_OBJECTS = $(patsubst $(DRIVERS_DIR)/%.c, $(BUILD_DIR)/drivers/%.o, $(DRIVER_SOURCES))
VGA_OBJECTS    = $(patsubst $(VGA_DIR)/%.c, $(BUILD_DIR)/drivers/vga/%.o, $(VGA_SOURCES))
MM_OBJECTS     = $(patsubst $(MM_DIR)/%.c, $(BUILD_DIR)/mm/%.o, $(MM_SOURCES))
FS_OBJECTS     = $(patsubst $(FS_DIR)/%.c, $(BUILD_DIR)/fs/%.o, $(FS_SOURCES))
LIB_OBJECTS    = $(patsubst $(LIB_DIR)/%.c, $(BUILD_DIR)/lib/%.o, $(LIB_SOURCES))
BOOT_OBJECTS   = $(patsubst $(BOOT_DIR)/%.s, $(BUILD_DIR)/boot/%.o, $(BOOT_SOURCES))
# --------------------------------------------
# Targets
# --------------------------------------------
.PHONY: all clean kernel bootloader qemu info help
all: kernel bootloader
# Create build directories
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)/kernel $(BUILD_DIR)/drivers $(BUILD_DIR)/drivers/vga $(BUILD_DIR)/mm $(BUILD_DIR)/fs $(BUILD_DIR)/lib $(BUILD_DIR)/boot
# --------------------------------------------
# Compilation
# --------------------------------------------
$(BUILD_DIR)/kernel/%.o: $(KERNEL_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/drivers/%.o: $(DRIVERS_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/drivers/vga/%.o: $(VGA_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/mm/%.o: $(MM_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/fs/%.o: $(FS_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/lib/%.o: $(LIB_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/boot/%.o: $(BOOT_DIR)/%.s | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@
# --------------------------------------------
# Kernel Build
# --------------------------------------------
kernel: $(KERNEL_OBJECTS) $(DRIVER_OBJECTS) $(VGA_OBJECTS) $(MM_OBJECTS) $(FS_OBJECTS) $(LIB_OBJECTS) $(BOOT_OBJECTS)
	$(LD) $(LDFLAGS) -T kernel/linker.ld -o $(BUILD_DIR)/kernel.elf $(KERNEL_OBJECTS) $(DRIVER_OBJECTS) $(VGA_OBJECTS) $(MM_OBJECTS) $(FS_OBJECTS) $(LIB_OBJECTS) $(BOOT_OBJECTS)
	$(OBJCOPY) -O binary $(BUILD_DIR)/kernel.elf $(BUILD_DIR)/kernel.bin
	@echo "✅ Kernel built for $(ARCH)$(BITS): $(BUILD_DIR)/kernel.bin"
# --------------------------------------------
# Bootloader Build
# --------------------------------------------
bootloader: $(BOOT_OBJECTS)
	$(LD) $(LDFLAGS) -T boot/linker.ld -o $(BUILD_DIR)/boot.elf $(BOOT_OBJECTS)
	$(OBJCOPY) -O binary $(BUILD_DIR)/boot.elf $(BUILD_DIR)/boot.bin
	@echo "✅ Bootloader built for $(ARCH)$(BITS): $(BUILD_DIR)/boot.bin"
# --------------------------------------------
# QEMU Run
# --------------------------------------------
qemu: kernel
ifeq ($(ARCH),x86)
	$(QEMU) -kernel $(BUILD_DIR)/kernel.bin -m 64M
else
	$(QEMU) -machine virt -kernel $(BUILD_DIR)/kernel.bin -m 128M -nographic
endif
# --------------------------------------------
# Clean
# --------------------------------------------
clean:
	rm -rf $(BUILD_DIR)
# --------------------------------------------
# Info
# --------------------------------------------
info:
	@echo "=== ACE Micro-Kernel Build Info ==="
	@echo "ARCH: $(ARCH)"
	@echo "BITS: $(BITS)"
	@echo "CC: $(CC)"
	@echo "CFLAGS: $(CFLAGS)"
	@echo "Kernel sources: $(KERNEL_SOURCES)"
	@echo "Boot sources: $(BOOT_SOURCES)"
# --------------------------------------------
# Help
# --------------------------------------------
help:
	@echo "Usage: make [TARGET] [ARCH=x86|riscv] [BITS=32|64]"
	@echo
	@echo "Targets:"
	@echo "  all         - Build kernel + bootloader"
	@echo "  kernel      - Build kernel only"
	@echo "  bootloader  - Build bootloader only"
	@echo "  qemu        - Run in QEMU"
	@echo "  clean       - Remove build artifacts"
	@echo "  info        - Show build info"
