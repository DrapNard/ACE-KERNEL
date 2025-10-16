#ifndef ARCH_X86_IO_H
#define ARCH_X86_IO_H

#include "ace/types.h"

// Port I/O functions
static inline void outb(u16 port, u8 val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline u8 inb(u16 port) {
    u8 ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

#endif /* ARCH_X86_IO_H */
