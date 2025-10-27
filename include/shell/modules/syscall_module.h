#ifndef SHELL_MODULES_SYSCALL_MODULE_H
#define SHELL_MODULES_SYSCALL_MODULE_H

#include "shell/shell.h"
#include "sys/syscalls.h"

// Syscall module functions
int syscall_module_init(void);
int syscall_module_cleanup(void);
int cmd_syscall(int argc, char* argv[]);
int cmd_sysinfo(int argc, char* argv[]);
int cmd_proc(int argc, char* argv[]);

// Syscall module structure
extern shell_module_t syscall_shell_module;

#endif /* SHELL_MODULES_SYSCALL_MODULE_H */