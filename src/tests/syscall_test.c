#include "tests/selftest.h"
#include "drivers/vga.h"
#include "sys/syscalls.h"

static void reset_params(syscall_params_t* params) {
    params->eax = 0;
    params->ebx = 0;
    params->ecx = 0;
    params->edx = 0;
    params->esi = 0;
    params->edi = 0;
}

void test_syscalls() {
    vga_print("Testing system calls...\n");
    
    syscall_params_t params;
    reset_params(&params);

    u32 pid = syscall_handler(SYSCALL_GETPID, &params);
    vga_printf("pid: %u\n", pid);

    reset_params(&params);
    params.ebx = 1;
    params.ecx = (u32)(uintptr_t)"Hello from syscall\n";
    params.edx = 19;
    syscall_handler(SYSCALL_WRITE, &params);

    reset_params(&params);
    params.ebx = 1024;
    u32 ptr = syscall_handler(SYSCALL_MALLOC, &params);
    if (ptr) {
        vga_print("Memory allocated\n");

        reset_params(&params);
        params.ebx = ptr;
        syscall_handler(SYSCALL_FREE, &params);
        vga_print("Memory freed\n");
    }

    reset_params(&params);
    params.ebx = 100;
    syscall_handler(SYSCALL_SLEEP, &params);
    
    vga_print("System calls test completed\n");
}
