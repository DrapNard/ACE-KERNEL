#include "../include/syscalls.h"
#include "../include/vga.h"

void test_syscalls() {
    vga_print("Testing system calls...\n");
    
    syscall_params_t params;
    
    params.eax = SYSCALL_GETPID;
    u32 pid = syscall_handler(SYSCALL_GETPID, &params);
    vga_print("PID: ");
    if (pid == 1) {
        vga_print("1\n");
    }
    
    params.eax = SYSCALL_WRITE;
    params.ebx = 1;
    params.ecx = (u32)"Hello from syscall\n";
    params.edx = 19;
    syscall_handler(SYSCALL_WRITE, &params);
    
    params.eax = SYSCALL_MALLOC;
    params.ebx = 1024;
    u32 ptr = syscall_handler(SYSCALL_MALLOC, &params);
    if (ptr) {
        vga_print("Memory allocated\n");
        
        params.eax = SYSCALL_FREE;
        params.ebx = ptr;
        syscall_handler(SYSCALL_FREE, &params);
        vga_print("Memory freed\n");
    }
    
    params.eax = SYSCALL_SLEEP;
    params.ebx = 100;
    syscall_handler(SYSCALL_SLEEP, &params);
    
    vga_print("System calls test completed\n");
}