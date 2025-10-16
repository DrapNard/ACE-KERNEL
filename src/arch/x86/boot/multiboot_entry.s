# Bootloader Multiboot compatible pour ACE Kernel
# Résout le problème "PVH ELF Note" de QEMU

.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot

# Multiboot header
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# Stack pour le kernel
.section .bss
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

# Point d'entrée du kernel
.section .text
.global _start
.type _start, @function
_start:
    # Configurer la pile
    mov $stack_top, %esp
    
    # Appeler le kernel principal
    call kernel_main
    
    # Si le kernel retourne, on s'arrête
    cli
1:  hlt
    jmp 1b

# Taille de _start pour le débogage
.size _start, . - _start