#include "../include/kernel.h"
#include "../include/vga.h"
#include "../include/memory.h"
#include "../include/scheduler.h"
#include "../include/interrupts.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("=== ACE Micro-Kernel v1.0 - Demo ===\n");
    printf("Initialisation...\n");

    memory_init();
    printf("Gestionnaire de memoire initialise\n");

    interrupts_init();
    printf("Interruptions initialisees\n");

    scheduler_init();
    printf("Ordonnanceur initialise\n");

    printf("Kernel pret\n");

    printf("\n=== Test des fonctionnalites ===\n");

    void* ptr1 = kmalloc(100);
    void* ptr2 = kmalloc(200);
    printf("Allocation memoire: ptr1=%p, ptr2=%p\n", ptr1, ptr2);

    memory_info();

    kfree(ptr1);
    kfree(ptr2);
    printf("Memoire liberee\n");

    printf("\nDemo terminee avec succes\n");
    return 0;
}

void kernel_shutdown(void) {
    printf("Système arrêté.\n");
    exit(0);
}

void kernel_panic(const char* message) {
    printf("\nKERNEL PANIC: %s\n", message);
    printf("Système arrêté.\n");
    exit(1);
}
