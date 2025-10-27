# ACE Kernel build system

ARCH ?= x86
BITS ?= 32

ifeq ($(ARCH),x86)
  ifeq ($(BITS),64)
    CROSS_COMPILE ?= x86_64-elf-
    QEMU ?= qemu-system-x86_64
    ARCH_CFLAGS := -m64
    ARCH_LDFLAGS := -m elf_x86_64
  else
    CROSS_COMPILE ?= i686-elf-
    QEMU ?= qemu-system-i386
    ARCH_CFLAGS := -m32
    ARCH_LDFLAGS := -m elf_i386
  endif
else
  $(error Unsupported ARCH $(ARCH))
endif

CC      := $(CROSS_COMPILE)gcc
LD      := $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy

BUILD_DIR    := build
TARGET_ELF   := $(BUILD_DIR)/kernel.elf
TARGET_BIN   := $(BUILD_DIR)/kernel.bin
MAP_FILE     := $(BUILD_DIR)/kernel.map

C_SOURCES      := $(shell find src -name '*.c')
ASM_UPPER_SRC  := $(shell find src -name '*.S')
ASM_LOWER_SRC  := $(shell find src -name '*.s')

C_OBJECTS      := $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(C_SOURCES))
ASM_OBJECTS    := $(patsubst src/%.S,$(BUILD_DIR)/%.o,$(ASM_UPPER_SRC)) \
                   $(patsubst src/%.s,$(BUILD_DIR)/%.o,$(ASM_LOWER_SRC))
OBJECTS     := $(C_OBJECTS) $(ASM_OBJECTS)

BOOT_OBJECT := $(BUILD_DIR)/arch/x86/boot/multiboot_entry.o

CFLAGS := $(ARCH_CFLAGS) -ffreestanding -fno-stack-protector -fno-pic \
          -fno-builtin -nostdlib -nostdinc -Wall -Wextra -Iinclude
LDFLAGS := $(ARCH_LDFLAGS)

ifeq ($(SELFTEST),1)
  CFLAGS += -DENABLE_KERNEL_SELFTESTS
endif

.PHONY: all clean run info

all: $(TARGET_BIN)

$(TARGET_BIN): $(TARGET_ELF)
	$(OBJCOPY) -O binary $< $@
	@echo "[build] kernel image -> $@"

$(TARGET_ELF): $(OBJECTS) linker.ld | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) -T linker.ld -Map $(MAP_FILE) -o $@ \
		$(BOOT_OBJECT) $(filter-out $(BOOT_OBJECT), $(OBJECTS))
	@echo "[link] $@"

$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: src/%.S | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: src/%.s | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

run: $(TARGET_ELF)
	$(QEMU) -kernel $(TARGET_ELF) -serial stdio -m 64M

info:
	@echo "ARCH      : $(ARCH)"
	@echo "BITS      : $(BITS)"
	@echo "CC        : $(CC)"
	@echo "CFLAGS    : $(CFLAGS)"
	@echo "Sources   : $(words $(C_SOURCES)) C, $(words $(ASM_UPPER_SRC) $(ASM_LOWER_SRC)) ASM"
	@echo "Selftests : $(SELFTEST)"

clean:
	rm -rf $(BUILD_DIR)