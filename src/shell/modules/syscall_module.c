#include "shell/modules/syscall_module.h"
#include "drivers/vga.h"
#include "sys/syscalls.h"
#include "shell/string_utils.h"

int syscall_module_init(void) {
    vga_print("[syscall_module] Initialisation\n");
    return 0;
}

int syscall_module_cleanup(void) {
    vga_print("[syscall_module] Nettoyage\n");
    return 0;
}

int cmd_syscall(int argc, char* argv[]) {
    if (argc < 2) {
        vga_print("Usage: syscall <num> [args...]\n");
        vga_print("Syscalls disponibles:\n");
        vga_print("  0 - exit(code)\n");
        vga_print("  1 - write(fd, buffer, count)\n");
        vga_print("  2 - read(fd, buffer, count)\n");
        vga_print("  3 - open(path, flags)\n");
        vga_print("  4 - close(fd)\n");
        vga_print("  8 - getpid()\n");
        vga_print("  9 - sleep(ms)\n");
        return -1;
    }

    u32 syscall_num = atoi(argv[1]);
    syscall_params_t params = {0};
    
    switch (syscall_num) {
        case SYSCALL_GETPID:
            params.eax = sys_getpid();
            vga_printf("PID: %u\n", params.eax);
            break;
            
        case SYSCALL_SLEEP:
            if (argc < 3) {
                vga_print("Usage: syscall 9 <milliseconds>\n");
                return -1;
            }
            params.ebx = atoi(argv[2]);
            params.eax = sys_sleep(params.ebx);
            vga_printf("Sleep termine\n");
            break;
            
        case SYSCALL_WRITE:
            if (argc < 4) {
                vga_print("Usage: syscall 1 <fd> <message>\n");
                return -1;
            }
            params.ebx = atoi(argv[2]);
            params.ecx = strlen(argv[3]);
            params.eax = sys_write(params.ebx, argv[3], params.ecx);
            vga_printf("Ecrit %u octets\n", params.eax);
            break;
            
        case SYSCALL_READ:
            if (argc < 4) {
                vga_print("Usage: syscall 2 <fd> <count>\n");
                return -1;
            }
            params.ebx = atoi(argv[2]);
            params.ecx = atoi(argv[3]);
            // For now, just allocate a temporary buffer on stack
            if (params.ecx > 256) params.ecx = 256; // Safety limit
            char temp_buffer[256];
            params.eax = sys_read(params.ebx, temp_buffer, params.ecx);
            if (params.eax > 0) {
                temp_buffer[params.eax] = '\0';
                vga_print("Lu: ");
                vga_print(temp_buffer);
                vga_print("\n");
            }
            break;
            
        case SYSCALL_OPEN:
            if (argc < 3) {
                vga_print("Usage: syscall 3 <path> [flags]\n");
                return -1;
            }
            params.ebx = 0; // Default flags (read-only)
            if (argc >= 4) {
                params.ebx = atoi(argv[3]);
            }
            params.eax = sys_open(argv[2], params.ebx);
            vga_printf("Fichier ouvert, fd: %u\n", params.eax);
            break;
            
        case SYSCALL_CLOSE:
            if (argc < 3) {
                vga_print("Usage: syscall 4 <fd>\n");
                return -1;
            }
            params.ebx = atoi(argv[2]);
            params.eax = sys_close(params.ebx);
            vga_printf("Fichier ferme, code: %u\n", params.eax);
            break;
            
        default:
            vga_printf("Syscall %u non supporte\n", syscall_num);
            return -1;
    }
    
    return 0;
}

int cmd_sysinfo(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    
    vga_print("=== Informations Systeme ===\n");
    vga_printf("PID courant: %u\n", sys_getpid());
    vga_printf("Temps de sommeil: %u ms\n", 0); // Would need actual timer
    vga_print("=== Fin Informations ===\n");
    return 0;
}

int cmd_proc(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    
    vga_print("=== Processus Actifs ===\n");
    vga_printf("PID: %u - kernel\n", 0);
    vga_printf("PID: %u - shell\n", sys_getpid());
    vga_printf("PID: %u - idle\n", 2);
    vga_print("=== Fin Processus ===\n");
    return 0;
}

// Module structure
shell_module_t syscall_shell_module = {
    .name = "syscall",
    .description = "Commandes de gestion des appels systeme",
    .init = syscall_module_init,
    .cleanup = syscall_module_cleanup,
    .next = NULL
};