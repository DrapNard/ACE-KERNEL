#ifndef SHELL_MODULES_FS_MODULE_H
#define SHELL_MODULES_FS_MODULE_H

#include "shell/shell.h"

// File system module functions
int fs_module_init(void);
int fs_module_cleanup(void);
int cmd_cp(int argc, char* argv[]);
int cmd_mv(int argc, char* argv[]);
int cmd_cd(int argc, char* argv[]);
int cmd_pwd(int argc, char* argv[]);

// File system module structure
extern shell_module_t fs_shell_module;

#endif /* SHELL_MODULES_FS_MODULE_H */