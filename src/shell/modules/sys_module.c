#include "shell/modules/sys_module.h"
#include "drivers/vga.h"
#include "core/kernel.h"

int sys_module_init(void) {
    vga_print("[sys_module] Initialisation\n");
    return 0;
}

int sys_module_cleanup(void) {
    vga_print("[sys_module] Nettoyage\n");
    return 0;
}

int cmd_reboot(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    vga_print("Redemarrage du systeme...\n");
    // In a real system, this would trigger a reboot
    // For now, just exit the shell
    return 1; // Exit shell
}

int cmd_shutdown(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    vga_print("Arret du systeme...\n");
    // In a real system, this would trigger a shutdown
    // For now, just exit the shell
    return 1; // Exit shell
}

int cmd_date(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    vga_print("Date: 2024-10-27\n");
    return 0;
}

int cmd_time(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    vga_print("Heure: 12:00:00\n");
    return 0;
}

// Module structure
shell_module_t sys_shell_module = {
    .name = "sys",
    .description = "Commandes systeme",
    .init = sys_module_init,
    .cleanup = sys_module_cleanup,
    .next = NULL
};