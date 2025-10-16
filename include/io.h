// io.h
#ifndef IO_H
#define IO_H

// Custom basic types (replace stdint.h)
typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;
typedef signed int      s32;

// Port I/O functions
static inline void outb(u16 port, u8 val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline u8 inb(u16 port) {
    u8 ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

#endif
