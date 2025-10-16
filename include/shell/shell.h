#ifndef SHELL_SHELL_H
#define SHELL_SHELL_H

#include "ace/types.h"

#define SHELL_BUFFER_SIZE 256
#define MAX_ARGS 16
#define MAX_COMMANDS 32

typedef struct {
    char name[32];
    char description[128];
    int (*handler)(int argc, char* argv[]);
} shell_command_t;

typedef struct {
    char buffer[SHELL_BUFFER_SIZE];
    u32 position;
    u32 history_index;
    char history[10][SHELL_BUFFER_SIZE];
} shell_state_t;

void shell_init();
void shell_run();
int  shell_execute_command(char* command);
void shell_register_command(char* name, char* description, int (*handler)(int argc, char* argv[]));
void shell_print_prompt();
void shell_clear_buffer();
int shell_parse_command(char* command, char* argv[]);

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

#endif /* SHELL_SHELL_H */
