/* Generic linker script for ACE Micro-Kernel */
/* Works for 32-bit and 64-bit kernels */

ENTRY(_start)

SECTIONS
{
    /* Load kernel at 1 MiB */
    . = 0x100000;

    /* Text (code) */
    .text ALIGN(4K) : {
        *(.multiboot)
        *(.text)
        *(.text.*)
    }

    /* Read-only data */
    .rodata ALIGN(4K) : {
        *(.rodata)
        *(.rodata.*)
    }

    /* Initialized data */
    .data ALIGN(4K) : {
        *(.data)
        *(.data.*)
    }

    /* Uninitialized data (BSS) */
    .bss ALIGN(4K) : {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }

    /* Kernel end marker */
    kernel_end = .;

    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
        *(.note*)
    }
}
