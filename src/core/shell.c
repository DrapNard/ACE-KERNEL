#include "shell/shell.h"
#include "shell/string_utils.h"
#include "drivers/vga.h"
#include "drivers/keyboard.h"
#include "fs/vfs.h"
#include "mm/heap.h"

static shell_state_t shell_state = {0};

static void normalize_path(const char* input, char* output) {
    u32 idx = 0;

    if (!input || !input[0]) {
        output[0] = '/';
        output[1] = 0;
        return;
    }

    if (input[0] != '/') {
        output[idx++] = '/';
    }

    for (u32 i = 0; input[i] && idx < MAX_PATH - 1; ++i) {
        output[idx++] = input[i];
    }

    output[idx] = 0;
}

void shell_init() {
    if (shell_state.initialized) return;
    
    // Initialize shell state
    shell_state.position = 0;
    shell_state.history_index = 0;
    shell_state.command_count = 0;
    shell_state.module_count = 0;
    shell_state.modules = NULL;

    for (int i = 0; i < 10; i++) {
        shell_state.history[i][0] = 0;
    }

    // Register built-in commands
    shell_register_command("help", "Affiche la liste des commandes", cmd_help, CMD_FLAG_INTERNAL);
    shell_register_command("clear", "Efface l'ecran", cmd_clear, CMD_FLAG_INTERNAL);
    shell_register_command("echo", "Affiche du texte", cmd_echo, CMD_FLAG_INTERNAL);
    shell_register_command("ls", "Liste les fichiers", cmd_ls, CMD_FLAG_INTERNAL);
    shell_register_command("cat", "Affiche le contenu d'un fichier", cmd_cat, CMD_FLAG_INTERNAL);
    shell_register_command("mkdir", "Cree un repertoire", cmd_mkdir, CMD_FLAG_INTERNAL);
    shell_register_command("touch", "Cree un fichier", cmd_touch, CMD_FLAG_INTERNAL);
    shell_register_command("rm", "Supprime un fichier", cmd_rm, CMD_FLAG_INTERNAL);
    shell_register_command("ps", "Liste les processus", cmd_ps, CMD_FLAG_INTERNAL);
    shell_register_command("kill", "Termine un processus", cmd_kill, CMD_FLAG_INTERNAL);
    shell_register_command("mem", "Affiche l'utilisation memoire", cmd_mem, CMD_FLAG_INTERNAL);
    shell_register_command("uptime", "Affiche le temps de fonctionnement", cmd_uptime, CMD_FLAG_INTERNAL);
    shell_register_command("exit", "Quitte le shell", cmd_exit, CMD_FLAG_INTERNAL);
    shell_register_command("modules", "Liste les modules shell", cmd_modules, CMD_FLAG_INTERNAL);

    shell_state.initialized = true;
    vga_print("Shell ACE initialise\n");
}

void shell_register_command(const char* name, const char* description, 
                           int (*handler)(int argc, char* argv[]), u32 flags) {
    if (!name || !handler || shell_state.command_count >= MAX_COMMANDS) return;

    u32 idx = shell_state.command_count;
    
    // Copy name
    u32 i = 0;
    while (name[i] && i < 31) {
        shell_state.commands[idx].name[i] = name[i];
        i++;
    }
    shell_state.commands[idx].name[i] = 0;

    // Copy description
    i = 0;
    while (description && description[i] && i < 127) {
        shell_state.commands[idx].description[i] = description[i];
        i++;
    }
    shell_state.commands[idx].description[i] = 0;

    shell_state.commands[idx].handler = handler;
    shell_state.commands[idx].flags = flags;
    shell_state.command_count++;
}

void shell_unregister_command(const char* name) {
    if (!name) return;

    for (u32 i = 0; i < shell_state.command_count; i++) {
        if (strings_equal(shell_state.commands[i].name, name)) {
            // Shift remaining commands down
            for (u32 j = i; j < shell_state.command_count - 1; j++) {
                shell_state.commands[j] = shell_state.commands[j + 1];
            }
            shell_state.command_count--;
            return;
        }
    }
}

int shell_register_module(shell_module_t* module) {
    if (!module || shell_state.module_count >= MAX_MODULES) return -1;

    // Add module to the linked list
    module->next = shell_state.modules;
    shell_state.modules = module;
    shell_state.module_count++;

    // Initialize the module
    if (module->init) {
        return module->init();
    }
    return 0;
}

int shell_unregister_module(const char* name) {
    if (!name) return -1;

    shell_module_t* current = shell_state.modules;
    shell_module_t* prev = NULL;

    while (current) {
        if (strings_equal(current->name, name)) {
            // Cleanup the module
            if (current->cleanup) {
                current->cleanup();
            }

            // Remove from linked list
            if (prev) {
                prev->next = current->next;
            } else {
                shell_state.modules = current->next;
            }

            shell_state.module_count--;
            return 0;
        }
        prev = current;
        current = current->next;
    }
    return -1;
}

void shell_print_prompt() {
    vga_print("ace@kernel:~$ ");
}

void shell_clear_buffer() {
    shell_state.position = 0;
    shell_state.buffer[0] = 0;
}

void shell_add_to_history(const char* command) {
    if (!command) return;
    
    u32 idx = shell_state.history_index % 10;
    copy_string(shell_state.history[idx], command, SHELL_BUFFER_SIZE);
    shell_state.history_index++;
}

void shell_show_history() {
    for (int i = 0; i < 10; i++) {
        if (shell_state.history[i][0] != 0) {
            vga_print(shell_state.history[i]);
            vga_print("\n");
        }
    }
}

void shell_run() {
    vga_print("\n=== Shell ACE ===\n");
    vga_print("Tapez 'help' pour voir les commandes disponibles\n");
    vga_print("Tapez 'modules' pour voir les modules disponibles\n\n");
    
    char input_buffer[256];
    
    while (1) {
        shell_print_prompt();
        
        int len = keyboard_readline(input_buffer, sizeof(input_buffer));
        
        if (len > 0) {
            vga_print("\n");
            shell_add_to_history(input_buffer);
            int rc = shell_execute_command(input_buffer);
            vga_print("\n");
            if (rc > 0) {
                break; // Exit requested
            }
        } else {
            vga_print("\n");
        }
    }

    vga_print("Retour au kernel...\n");
}

int shell_execute_command(char* command) {
    if (!command || command[0] == 0) {
        return 0;
    }

    char* argv[MAX_ARGS];
    const int argc = shell_parse_command(command, argv);
    if (argc == 0) {
        return 0;
    }

    // Search for command in registered commands
    for (u32 i = 0; i < shell_state.command_count; i++) {
        if (strings_equal(argv[0], shell_state.commands[i].name)) {
            return shell_state.commands[i].handler(argc, argv);
        }
    }

    vga_print("Commande inconnue: ");
    vga_print(argv[0]);
    vga_print("\n");
    return 0;
}

int shell_parse_command(char* command, char* argv[]) {
    int argc = 0;
    int i = 0;

    while (command[i] && argc < MAX_ARGS - 1) {
        while (command[i] == ' ') i++;
        if (!command[i]) break;

        argv[argc] = &command[i];
        argc++;

        while (command[i] && command[i] != ' ') i++;
        if (command[i]) {
            command[i] = 0;
            i++;
        }
    }

    argv[argc] = 0;
    return argc;
}

// Built-in command implementations
int cmd_help(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    vga_print("Commandes disponibles:\n");
    for (u32 i = 0; i < shell_state.command_count; i++) {
        // Skip hidden commands
        if (shell_state.commands[i].flags & CMD_FLAG_HIDDEN) continue;
        
        vga_print("  ");
        vga_print(shell_state.commands[i].name);
        vga_print(" - ");
        vga_print(shell_state.commands[i].description);
        vga_print("\n");
    }
    return 0;
}

int cmd_clear(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    vga_clear();
    return 0;
}

int cmd_echo(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        vga_print(argv[i]);
        if (i < argc - 1) vga_print(" ");
    }
    vga_print("\n");
    return 0;
}

int cmd_ls(int argc, char* argv[]) {
    char path[MAX_PATH];
    normalize_path((argc > 1) ? argv[1] : "/", path);

    vfs_node_t* node = vfs_resolve_path(path);
    if (!node) {
        vga_printf("Chemin introuvable: %s\n", path);
        return -1;
    }

    if (node->type != FILE_TYPE_DIRECTORY) {
        vga_printf("%s\n", node->name);
        return 0;
    }

    vga_printf("Liste de %s:\n", path);
    vfs_node_t* child = node->children;
    if (!child) {
        vga_print("  <vide>\n");
        return 0;
    }

    while (child) {
        vga_print("  ");
        vga_print(child->name);
        if (child->type == FILE_TYPE_DIRECTORY) {
            vga_print("/");
        }
        vga_print("\n");
        child = child->next;
    }

    return 0;
}

int cmd_cat(int argc, char* argv[]) {
    if (argc < 2) {
        vga_print("Usage: cat <fichier>\n");
        return -1;
    }
    char path[MAX_PATH];
    normalize_path(argv[1], path);

    int fd = vfs_open(path, O_RDONLY);
    if (fd < 0) {
        vga_printf("Impossible d'ouvrir %s\n", path);
        return -1;
    }

    char buffer[128];
    int read = vfs_read(fd, buffer, sizeof(buffer) - 1);
    while (read > 0) {
        buffer[read] = '\0';
        vga_print(buffer);
        read = vfs_read(fd, buffer, sizeof(buffer) - 1);
    }

    if (read < 0) {
        vga_print("\nErreur de lecture\n");
    }

    vfs_close(fd);
    vga_print("\n");
    return 0;
}

int cmd_mkdir(int argc, char* argv[]) {
    if (argc < 2) {
        vga_print("Usage: mkdir <repertoire>\n");
        return -1;
    }
    char path[MAX_PATH];
    normalize_path(argv[1], path);

    if (vfs_mkdir(path) == 0) {
        vga_printf("Repertoire cree: %s\n", path);
        return 0;
    }

    vga_printf("Echec creation: %s\n", path);
    return -1;
}

int cmd_touch(int argc, char* argv[]) {
    if (argc < 2) {
        vga_print("Usage: touch <fichier>\n");
        return -1;
    }
    char path[MAX_PATH];
    normalize_path(argv[1], path);

    int fd = vfs_open(path, O_CREAT | O_RDWR);
    if (fd < 0) {
        vga_printf("Impossible de creer: %s\n", path);
        return -1;
    }
    vfs_close(fd);
    vga_printf("Fichier cree: %s\n", path);
    return 0;
}

int cmd_rm(int argc, char* argv[]) {
    if (argc < 2) {
        vga_print("Usage: rm <fichier>\n");
        return -1;
    }
    char path[MAX_PATH];
    normalize_path(argv[1], path);

    if (vfs_unlink(path) == 0) {
        vga_printf("Supprime: %s\n", path);
        return 0;
    }

    vga_printf("Echec suppression: %s\n", path);
    return -1;
}

int cmd_ps(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    vga_print("PID  PPID CMD\n");
    vga_print("  1     0 kernel\n");
    vga_print("  2     1 shell\n");
    vga_print("  3     1 idle\n");
    return 0;
}

int cmd_kill(int argc, char* argv[]) {
    if (argc < 2) {
        vga_print("Usage: kill <pid>\n");
        return -1;
    }

    vga_print("Signal envoye au processus ");
    vga_print(argv[1]);
    vga_print("\n");
    return 0;
}

int cmd_mem(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    memory_info();
    return 0;
}

int cmd_uptime(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    vga_print("Temps de fonctionnement: 0 jours, 0 heures, 5 minutes\n");
    return 0;
}

int cmd_exit(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    vga_print("Arret du shell...\n");
    return 1; // Return 1 to exit shell
}

int cmd_modules(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    vga_print("Modules shell charges:\n");
    shell_module_t* current = shell_state.modules;
    u32 count = 0;
    
    while (current) {
        vga_print("  [");
        vga_print(current->name);
        vga_print("] - ");
        vga_print(current->description);
        vga_print("\n");
        current = current->next;
        count++;
    }
    
    if (count == 0) {
        vga_print("  Aucun module charge\n");
    }
    
    vga_printf("Total: %u modules\n", count);
    return 0;
}