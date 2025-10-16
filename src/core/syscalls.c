#include "sys/syscalls.h"
#include "core/scheduler.h"
#include "drivers/vga.h"
#include "fs/vfs.h"
#include "mm/heap.h"

void syscall_init() {
    vga_print("Syscalls initialized\n");
}

u32 syscall_handler(u32 syscall_num, syscall_params_t* params) {
    switch (syscall_num) {
        case SYSCALL_EXIT:
            return sys_exit(params->ebx);
        case SYSCALL_WRITE:
            return sys_write(params->ebx, (const char*)params->ecx, params->edx);
        case SYSCALL_READ:
            return sys_read(params->ebx, (char*)params->ecx, params->edx);
        case SYSCALL_OPEN:
            return sys_open((const char*)params->ebx, params->ecx);
        case SYSCALL_CLOSE:
            return sys_close(params->ebx);
        case SYSCALL_FORK:
            return sys_fork();
        case SYSCALL_EXEC:
            return sys_exec((const char*)params->ebx);
        case SYSCALL_WAIT:
            return sys_wait((u32*)params->ebx);
        case SYSCALL_GETPID:
            return sys_getpid();
        case SYSCALL_SLEEP:
            return sys_sleep(params->ebx);
        case SYSCALL_MALLOC:
            return sys_malloc(params->ebx);
        case SYSCALL_FREE:
            return sys_free(params->ebx);
        default:
            return -1;
    }
}

u32 sys_exit(u32 code) {
    (void)code;
    vga_print("Process exit\n");
    return 0;
}

u32 sys_write(u32 fd, const char* buffer, u32 count) {
    if (fd == 1 || fd == 2) {
        for (u32 i = 0; i < count; i++) {
            vga_putchar(buffer[i]);
        }
        return count;
    }
    int written = vfs_write(fd, (void*)buffer, count);
    return written < 0 ? (u32)-1 : (u32)written;
}

u32 sys_read(u32 fd, char* buffer, u32 count) {
    if (fd == 0) {
        for (u32 i = 0; i < count; i++) {
            buffer[i] = 'A' + (i % 26);
        }
        return count;
    }
    int read = vfs_read(fd, buffer, count);
    return read < 0 ? (u32)-1 : (u32)read;
}

u32 sys_open(const char* path, u32 flags) {
    int fd = vfs_open((char*)path, flags);
    return fd < 0 ? (u32)-1 : (u32)fd;
}

u32 sys_close(u32 fd) {
    return (u32)vfs_close(fd);
}

u32 sys_fork() {
    vga_print("Fork not implemented\n");
    return -1;
}

u32 sys_exec(const char* path) {
    (void)path;
    return -1;
}

u32 sys_wait(u32* status) {
    vga_print("Wait not implemented\n");
    if (status) *status = 0;
    return 0;
}

u32 sys_getpid() {
    return 1;
}

u32 sys_sleep(u32 ms) {
    (void)ms;
    vga_print("Sleep called\n");
    return 0;
}

u32 sys_malloc(u32 size) {
    void* ptr = kmalloc(size);
    return (u32)ptr;
}

u32 sys_free(u32 ptr) {
    if (ptr) {
        kfree((void*)ptr);
    }
    return 0;
}

void syscall_entry() {
    vga_print("Syscall entry point\n");
}

void syscall_dispatcher(u32* stack) {
    (void)stack;
    vga_print("Syscall dispatcher\n");
}
