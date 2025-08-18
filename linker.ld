/* Script de liaison pour ACE Micro-Kernel */

ENTRY(kernel_main)

SECTIONS
{
    /* Le kernel commence à 1MB (0x100000) */
    . = 0x100000;
    
    /* Section de code */
    .text ALIGN(4K) : {
        *(.text)
    }
    
    /* Section de données en lecture seule */
    .rodata ALIGN(4K) : {
        *(.rodata)
    }
    
    /* Section de données */
    .data ALIGN(4K) : {
        *(.data)
    }
    
    /* Section BSS (données non initialisées) */
    .bss ALIGN(4K) : {
        *(COMMON)
        *(.bss)
    }
    
    /* Fin du kernel */
    kernel_end = .;
}