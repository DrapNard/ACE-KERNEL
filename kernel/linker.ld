/* kernel/linker.ld - Kernel linker script for ACE Micro-Kernel */

ENTRY(kernel_main)

SECTIONS
{
    /* Kernel starts at 1 MB */
    . = 0x100000;

    /* Code section */
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

    /* BSS (zero-initialized data) */
    .bss ALIGN(4K) : {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }

    kernel_end = .;

    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
        *(.note*)
    }
}
