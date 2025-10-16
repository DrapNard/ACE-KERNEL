#include "libk/mem.h"

void* k_memcpy(void* dest, const void* src, size_t count) {
    u8* d = (u8*)dest;
    const u8* s = (const u8*)src;
    while (count--) {
        *d++ = *s++;
    }
    return dest;
}

void* k_memmove(void* dest, const void* src, size_t count) {
    u8* d = (u8*)dest;
    const u8* s = (const u8*)src;

    if (d == s || count == 0) {
        return dest;
    }

    if (d < s) {
        while (count--) {
            *d++ = *s++;
        }
    } else {
        d += count;
        s += count;
        while (count--) {
            *--d = *--s;
        }
    }

    return dest;
}

void* k_memset(void* dest, int value, size_t count) {
    u8* ptr = (u8*)dest;
    while (count--) {
        *ptr++ = (u8)value;
    }
    return dest;
}

int k_memcmp(const void* lhs, const void* rhs, size_t count) {
    const u8* a = (const u8*)lhs;
    const u8* b = (const u8*)rhs;
    while (count--) {
        if (*a != *b) {
            return (int)(*a - *b);
        }
        ++a;
        ++b;
    }
    return 0;
}
