# Makefile pour ACE Micro-Kernel

# Compilateur et assembleur (version macOS compatible)
CC = gcc
AS = as
LD = ld
OBJCOPY = objcopy

# Flags de compilation (version simplifiée pour macOS)
CFLAGS = -fno-builtin -fno-stack-protector \
         -Wall -Wextra -c
ASFLAGS = 
LDFLAGS = 

# Répertoires
KERNEL_DIR = kernel
BOOT_DIR = boot
DRIVERS_DIR = drivers
MM_DIR = mm
INCLUDE_DIR = include
BUILD_DIR = build

# Fichiers sources
KERNEL_SOURCES = $(wildcard kernel/*.c)
DRIVER_SOURCES = $(wildcard drivers/*.c)
MM_SOURCES = $(wildcard mm/*.c)
FS_SOURCES = $(wildcard fs/*.c)
LIB_SOURCES = $(wildcard lib/*.c)
BOOT_SOURCES = $(wildcard boot/*.s)

# Fichiers objets pour macOS
KERNEL_OBJECTS = $(filter-out $(BUILD_DIR)/kernel_qemu.o, $(KERNEL_SOURCES:kernel/%.c=$(BUILD_DIR)/%.o))
DRIVER_OBJECTS = $(DRIVER_SOURCES:drivers/%.c=$(BUILD_DIR)/%.o)
MM_OBJECTS = $(MM_SOURCES:mm/%.c=$(BUILD_DIR)/%.o)
FS_OBJECTS = $(FS_SOURCES:fs/%.c=$(BUILD_DIR)/%.o)
LIB_OBJECTS = $(LIB_SOURCES:lib/%.c=$(BUILD_DIR)/%.o)

# Fichiers objets pour QEMU
QEMU_KERNEL_OBJECTS = $(BUILD_DIR)/kernel_qemu.o $(DRIVER_OBJECTS) $(MM_OBJECTS) $(FS_OBJECTS) $(LIB_OBJECTS)
BOOT_OBJECTS = $(BOOT_SOURCES:boot/%.s=$(BUILD_DIR)/%.o)

ALL_OBJECTS = $(KERNEL_OBJECTS) $(DRIVER_OBJECTS) $(MM_OBJECTS) $(FS_OBJECTS) $(LIB_OBJECTS)

# Cibles principales
.PHONY: all clean kernel kernel-qemu bootloader qemu run info help

all: kernel

# Créer le répertoire de build
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compiler les fichiers du kernel
$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) $< -o $@

# Compiler les drivers
$(BUILD_DIR)/%.o: $(DRIVERS_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) $< -o $@

# Compiler la gestion mémoire
$(BUILD_DIR)/%.o: $(MM_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) $< -o $@

# Objets pour QEMU
$(BUILD_DIR)/kernel_qemu.o: kernel/kernel_qemu.c | $(BUILD_DIR)
	x86_64-elf-gcc $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

$(BUILD_DIR)/vga.o: drivers/vga.c | $(BUILD_DIR)
	x86_64-elf-gcc $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

$(BUILD_DIR)/memory.o: mm/memory.c | $(BUILD_DIR)
	x86_64-elf-gcc $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Créer le kernel (version simplifiée)
kernel: $(ALL_OBJECTS) | $(BUILD_DIR)
	$(CC) $(ALL_OBJECTS) -o $(BUILD_DIR)/kernel.bin

# Assembler le bootloader
$(BUILD_DIR)/boot.o: boot/boot.s | $(BUILD_DIR)
	x86_64-elf-as --32 $< -o $@

# Créer le kernel pour QEMU
kernel-qemu: $(QEMU_KERNEL_OBJECTS) | $(BUILD_DIR)
	x86_64-elf-gcc -m32 -nostdlib -T linker.ld $(QEMU_KERNEL_OBJECTS) -o $(BUILD_DIR)/kernel_qemu.bin

# Créer le bootloader
bootloader: $(BUILD_DIR)/boot.o | $(BUILD_DIR)
	x86_64-elf-objcopy -O binary $(BUILD_DIR)/boot.o $(BUILD_DIR)/boot.bin

# Compiler le kernel simple pour QEMU
$(BUILD_DIR)/kernel_simple.o: kernel/kernel_simple.c | $(BUILD_DIR)
	x86_64-elf-gcc -m32 -fno-builtin -fno-stack-protector -nostdlib -c $< -o $@

$(BUILD_DIR)/syscalls.o: kernel/syscalls.c | $(BUILD_DIR)
	x86_64-elf-gcc -m32 -fno-builtin -fno-stack-protector -nostdlib -c $< -o $@

$(BUILD_DIR)/syscall_test.o: kernel/syscall_test.c | $(BUILD_DIR)
	x86_64-elf-gcc -m32 -fno-builtin -fno-stack-protector -nostdlib -c $< -o $@

$(BUILD_DIR)/vfs.o: fs/vfs.c | $(BUILD_DIR)
	x86_64-elf-gcc -m32 -fno-builtin -fno-stack-protector -nostdlib -c $< -o $@

$(BUILD_DIR)/vfs_test.o: fs/vfs_test.c | $(BUILD_DIR)
	x86_64-elf-gcc -m32 -fno-builtin -fno-stack-protector -nostdlib -c $< -o $@

$(BUILD_DIR)/shell.o: kernel/shell.c | $(BUILD_DIR)
	x86_64-elf-gcc -m32 -fno-builtin -fno-stack-protector -nostdlib -c $< -o $@

$(BUILD_DIR)/keyboard.o: drivers/keyboard.c | $(BUILD_DIR)
	x86_64-elf-gcc -m32 -fno-builtin -fno-stack-protector -nostdlib -c $< -o $@

$(BUILD_DIR)/interrupts.o: kernel/interrupts.c | $(BUILD_DIR)
	x86_64-elf-gcc -m32 -fno-builtin -fno-stack-protector -nostdlib -c $< -o $@

$(BUILD_DIR)/kernel_simple.elf: $(BUILD_DIR)/kernel_simple.o $(BUILD_DIR)/syscalls.o $(BUILD_DIR)/syscall_test.o $(BUILD_DIR)/shell.o $(BUILD_DIR)/keyboard.o $(BUILD_DIR)/interrupts.o | $(BUILD_DIR)
	x86_64-elf-ld -m elf_i386 -T multiboot.ld -o $@ $^

$(BUILD_DIR)/kernel_simple.bin: $(BUILD_DIR)/kernel_simple.elf | $(BUILD_DIR)
	x86_64-elf-objcopy -O binary $< $@

# Compiler le bootloader Multiboot
$(BUILD_DIR)/multiboot_boot.o: boot/multiboot_boot.s | $(BUILD_DIR)
	x86_64-elf-as --32 $< -o $@

# Créer le kernel avec bootloader intégré
$(BUILD_DIR)/kernel_multiboot.elf: $(BUILD_DIR)/multiboot_boot.o $(BUILD_DIR)/kernel_simple.o $(BUILD_DIR)/syscalls.o $(BUILD_DIR)/syscall_test.o $(BUILD_DIR)/shell.o $(BUILD_DIR)/keyboard.o $(BUILD_DIR)/interrupts.o | $(BUILD_DIR)
	x86_64-elf-ld -m elf_i386 -T multiboot.ld -o $@ $^

# Créer l'image ISO bootable
iso: $(BUILD_DIR)/kernel_multiboot.elf | $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/isodir/boot/grub
	cp $< $(BUILD_DIR)/isodir/boot/kernel.elf
	echo 'set timeout=0' > $(BUILD_DIR)/isodir/boot/grub/grub.cfg
	echo 'set default=0' >> $(BUILD_DIR)/isodir/boot/grub/grub.cfg
	echo '' >> $(BUILD_DIR)/isodir/boot/grub/grub.cfg
	echo 'menuentry "ACE Kernel" {' >> $(BUILD_DIR)/isodir/boot/grub/grub.cfg
	echo '    multiboot /boot/kernel.elf' >> $(BUILD_DIR)/isodir/boot/grub/grub.cfg
	echo '    boot' >> $(BUILD_DIR)/isodir/boot/grub/grub.cfg
	echo '}' >> $(BUILD_DIR)/isodir/boot/grub/grub.cfg
	i686-elf-grub-mkrescue -o $(BUILD_DIR)/ace_kernel.iso $(BUILD_DIR)/isodir

# Lancer QEMU avec le kernel Multiboot
qemu: $(BUILD_DIR)/kernel_multiboot.elf
	qemu-system-i386 -kernel $< -nographic

# Lancer QEMU avec l'image ISO
qemu-iso: $(BUILD_DIR)/ace_kernel.iso
	qemu-system-i386 -cdrom $< -nographic

# Version simplifiée - exécuter directement sur macOS
run: kernel
	$(BUILD_DIR)/kernel.bin

# Nettoyer les fichiers générés
clean:
	rm -rf $(BUILD_DIR)

# Afficher les informations de build
info:
	@echo "=== ACE Micro-Kernel Build Info ==="
	@echo "Compilateur: $(CC)"
	@echo "Flags: $(CFLAGS)"
	@echo "Sources kernel: $(KERNEL_SOURCES)"
	@echo "Sources drivers: $(DRIVERS_SOURCES)"
	@echo "Sources MM: $(MM_SOURCES)"
	@echo "Objets: $(ALL_OBJECTS)"

# Aide
help:
	@echo "Cibles disponibles:"
	@echo "  all        - Compiler le kernel et le bootloader"
	@echo "  kernel     - Compiler seulement le kernel"
	@echo "  bootloader - Compiler seulement le bootloader"
	@echo "  iso        - Créer une image disque bootable"
	@echo "  run        - Exécuter avec QEMU (si disponible)"
	@echo "  clean      - Nettoyer les fichiers générés"
	@echo "  info       - Afficher les informations de build"
	@echo "  help       - Afficher cette aide"