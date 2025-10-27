#ifndef SHELL_MODULES_SYS_MODULE_H
#define SHELL_MODULES_SYS_MODULE_H

#include "shell/shell.h"

// System module functions
int sys_module_init(void);
int sys_module_cleanup(void);
int cmd_reboot(int argc, char* argv[]);
int cmd_shutdown(int argc, char* argv[]);
int cmd_date(int argc, char* argv[]);
int cmd_time(int argc, char* argv[]);

// System module structure
extern shell_module_t sys_shell_module;

#endif /* SHELL_MODULES_SYS_MODULE_H */