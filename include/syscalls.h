#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "kernel.h"

#define SYSCALL_EXIT     0
#define SYSCALL_WRITE    1
#define SYSCALL_READ     2
#define SYSCALL_OPEN     3
#define SYSCALL_CLOSE    4
#define SYSCALL_FORK     5
#define SYSCALL_EXEC     6
#define SYSCALL_WAIT     7
#define SYSCALL_GETPID   8
#define SYSCALL_SLEEP    9
#define SYSCALL_MALLOC   10
#define SYSCALL_FREE     11

typedef struct {
    u32 eax;
    u32 ebx;
    u32 ecx;
    u32 edx;
    u32 esi;
    u32 edi;
} syscall_params_t;

void syscall_init();
u32 syscall_handler(u32 syscall_num, syscall_params_t* params);

u32 sys_exit(u32 code);
u32 sys_write(u32 fd, const char* buffer, u32 count);
u32 sys_read(u32 fd, char* buffer, u32 count);
u32 sys_open(const char* path, u32 flags);
u32 sys_close(u32 fd);
u32 sys_fork();
u32 sys_exec(const char* path);
u32 sys_wait(u32* status);
u32 sys_getpid();
u32 sys_sleep(u32 ms);
u32 sys_malloc(u32 size);
u32 sys_free(u32 ptr);

#endif