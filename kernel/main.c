#include "../include/kernel.h"
#include "../drivers/vga/vga.h"
#include "../include/memory.h"
#include "../include/scheduler.h"
#include "../include/interrupts.h"

int main(void) {
    vga_printf("=== ACE Micro-Kernel v1.0 - Demo ===\n");
    vga_printf("Initialisation...\n");

    memory_init();
    vga_printf("Gestionnaire de memoire initialise\n");

    interrupts_init();
    vga_printf("Interruptions initialisees\n");

    scheduler_init();
    vga_printf("Ordonnanceur initialise\n");

    vga_printf("Kernel pret\n");

    vga_printf("\n=== Test des fonctionnalites ===\n");

    void* ptr1 = kmalloc(100);
    void* ptr2 = kmalloc(200);
    vga_printf("Allocation memoire: ptr1=%p, ptr2=%p\n", ptr1, ptr2);

    memory_info();

    kfree(ptr1);
    kfree(ptr2);
    vga_printf("Memoire liberee\n");

    vga_printf("\nDemo terminee avec succes\n");
    return 0;
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