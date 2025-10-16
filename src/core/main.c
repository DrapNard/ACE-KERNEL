#include "core/kernel.h"
#include "core/interrupts.h"
#include "core/scheduler.h"
#include "drivers/keyboard.h"
#include "drivers/vga.h"
#include "fs/vfs.h"
#include "mm/heap.h"
#include "shell/shell.h"
#include "sys/syscalls.h"
#include "tests/selftest.h"

static void kernel_banner(void) {
    vga_clear();
    VGA_COLOR_INFO();
    vga_printf("========================================\n");
    vga_printf("  %s (%s)\n", KERNEL_NAME, KERNEL_VERSION);
    vga_printf("  Initialisation sequence starting...\n");
    vga_printf("========================================\n\n");
    VGA_COLOR_DEFAULT();
}

static void kernel_ready(void) {
    VGA_COLOR_OK();
    vga_printf("\nKernel initialisation completed.\n");
    vga_printf("Launching interactive shell...\n\n");
    VGA_COLOR_DEFAULT();
}

void kernel_main(void) {
    vga_init();
    kernel_banner();

    memory_init();
    vga_printf("[mem] heap initialised (%u bytes)\n", KERNEL_HEAP_SIZE);

    interrupts_init();
    vga_printf("[int] IDT configured\n");

    scheduler_init();
    vga_printf("[sch] scheduler ready\n");

    syscall_init();
    vga_printf("[sys] syscall interface online\n");

    vfs_init();
    vga_printf("[fs] virtual filesystem ready\n");

    keyboard_init();
    vga_printf("[drv] keyboard controller registered\n");

#ifdef ENABLE_KERNEL_SELFTESTS
    vga_printf("\nRunning kernel self-tests...\n");
    test_syscalls();
    test_vfs();
    vga_printf("Self-tests completed.\n");
#endif

    shell_init();
    kernel_ready();

    shell_run();
    kernel_shutdown();
}

void kernel_shutdown(void) {
    VGA_COLOR_WARN();
    vga_print("\nKernel shutdown requested. Halting CPU.\n");
    VGA_COLOR_DEFAULT();
    while (1) {
        asm volatile ("hlt");
    }
}

void kernel_panic(const char* message) {
    VGA_COLOR_ERROR();
    vga_print("\nKERNEL PANIC: ");
    vga_print(message);
    vga_print("\nSystem halted.\n");
    VGA_COLOR_DEFAULT();
    while (1) {
        asm volatile ("hlt");
    }
}
