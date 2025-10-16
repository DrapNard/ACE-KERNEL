/* boot/linker.ld - Bootloader linker script for ACE */

ENTRY(_start)

SECTIONS
{
    /* Bootloader starts at 0x7C00 */
    . = 0x7C00;

    .text ALIGN(512) : {
        *(.text)
        *(.text.*)
    }

    .rodata ALIGN(512) : {
        *(.rodata)
        *(.rodata.*)
    }

    .data ALIGN(512) : {
        *(.data)
        *(.data.*)
    }

    .bss ALIGN(512) : {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }

    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
        *(.note*)
    }
}
