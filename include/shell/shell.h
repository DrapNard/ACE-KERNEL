#ifndef SHELL_SHELL_H
#define SHELL_SHELL_H

#include "ace/types.h"

#define SHELL_BUFFER_SIZE 256
#define MAX_ARGS 16
#define MAX_COMMANDS 32
#define MAX_MODULES 16

// Shell command structure
typedef struct {
    char name[32];
    char description[128];
    int (*handler)(int argc, char* argv[]);
    u32 flags;  // Command flags
} shell_command_t;

// Shell module structure
typedef struct shell_module {
    char name[32];
    char description[128];
    int (*init)(void);
    int (*cleanup)(void);
    struct shell_module* next;
} shell_module_t;

// Shell state structure
typedef struct {
    char buffer[SHELL_BUFFER_SIZE];
    u32 position;
    u32 history_index;
    char history[10][SHELL_BUFFER_SIZE];
    shell_command_t commands[MAX_COMMANDS];
    u32 command_count;
    shell_module_t* modules;
    u32 module_count;
    bool initialized;
} shell_state_t;

// Command flags
#define CMD_FLAG_INTERNAL    0x01  // Internal command (not external)
#define CMD_FLAG_PRIVILEGED  0x02  // Requires elevated privileges
#define CMD_FLAG_HIDDEN      0x04  // Don't show in help

// Core shell functions
void shell_init(void);
void shell_run(void);
int  shell_execute_command(char* command);
void shell_register_command(const char* name, const char* description, 
                           int (*handler)(int argc, char* argv[]), u32 flags);
void shell_unregister_command(const char* name);
void shell_print_prompt(void);
void shell_clear_buffer(void);
int  shell_parse_command(char* command, char* argv[]);
void shell_add_to_history(const char* command);
void shell_show_history(void);

// Module management functions
int  shell_register_module(shell_module_t* module);
int  shell_unregister_module(const char* name);
void shell_list_modules(void);

// Built-in command functions
int cmd_help(int argc, char* argv[]);
int cmd_clear(int argc, char* argv[]);
int cmd_echo(int argc, char* argv[]);
int cmd_ls(int argc, char* argv[]);
int cmd_cat(int argc, char* argv[]);
int cmd_mkdir(int argc, char* argv[]);
int cmd_touch(int argc, char* argv[]);
int cmd_rm(int argc, char* argv[]);
int cmd_ps(int argc, char* argv[]);
int cmd_kill(int argc, char* argv[]);
int cmd_mem(int argc, char* argv[]);
int cmd_uptime(int argc, char* argv[]);
int cmd_exit(int argc, char* argv[]);
int cmd_modules(int argc, char* argv[]);

// Add these function declarations to shell.h
static inline int strings_equal(const char* a, const char* b) {
    if (!a || !b) {
        return 0;
    }
    size_t i = 0;
    while (a[i] || b[i]) {
        if (a[i] != b[i]) {
            return 0;
        }
        ++i;
    }
    return 1;
}

static inline void copy_string(char* dest, const char* src, size_t max_len) {
    if (!dest || !max_len) {
        return;
    }
    size_t i = 0;
    if (src) {
        for (; i + 1 < max_len && src[i]; ++i) {
            dest[i] = src[i];
        }
    }
    dest[i] = '\0';
}

#endif /* SHELL_SHELL_H */