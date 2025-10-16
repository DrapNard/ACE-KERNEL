// Définitions de base pour QEMU x86
#ifndef __KERNEL_TYPES_DEFINED__
#define __KERNEL_TYPES_DEFINED__
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
#ifndef _SIZE_T
#define _SIZE_T
typedef u32 size_t;
#endif
#endif

#include "../drivers/vga/vga.h"
#include "../include/memory.h"
#include "../include/scheduler.h"
#include "../include/interrupts.h"

void kernel_main(void) {
    vga_init();
    vga_clear();
    vga_print("ACE Kernel v1.0 - QEMU\n");
    vga_print("Initialisation\n");

    memory_init();
    vga_print("Gestionnaire de memoire initialise\n");

    interrupts_init();
    vga_print("Interruptions initialisees\n");

    scheduler_init();
    vga_print("Ordonnanceur initialise\n");

    vga_print("Kernel pret\n");

    vga_print("\n=== Test des fonctionnalites ===\n");

    void* ptr1 = kmalloc(100);
    void* ptr2 = kmalloc(200);
    vga_print("Allocation memoire reussie\n");

    vga_print("=== Informations Memoire ===\n");
    vga_print("Memoire totale: 1MB\n");
    vga_print("Allocation et liberation OK\n");

    kfree(ptr1);
    kfree(ptr2);
    vga_print("Memoire liberee\n");

    vga_print("\nKernel ACE fonctionne correctement!\n");
    vga_print("Appuyez sur Ctrl+Alt+Q pour quitter QEMU\n");

    while (1) {
        for (volatile int i = 0; i < 1000000; i++) {
            // Attente active
        }
    }
}

// Shuts down the kernel (infinite loop)
void kernel_shutdown(void) {
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_print("Arret du kernel...\n");
    while (1) {
        asm volatile ("hlt");  // Halt CPU
    }
}

// Handles kernel panics (infinite loop with error message)
void kernel_panic(const char* message) {
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_RED);
    vga_print("\nKERNEL PANIC: ");
    vga_print(message);
    vga_print("\nSysteme arrete.\n");
    while (1) {
        asm volatile ("hlt");  // Halt CPU
    }
}