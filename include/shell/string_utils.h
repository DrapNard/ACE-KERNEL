#ifndef SHELL_STRING_UTILS_H
#define SHELL_STRING_UTILS_H

#include "ace/types.h"

// String utility functions for shell
static inline size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

static inline int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

static inline char* strcat(char* dest, const char* src) {
    char* ret = dest;
    while (*dest) dest++;
    while ((*dest++ = *src++));
    return ret;
}

static inline char* strncat(char* dest, const char* src, size_t n) {
    char* ret = dest;
    while (*dest) dest++;
    size_t i = 0;
    while (i < n && src[i]) {
        *dest++ = src[i++];
    }
    *dest = 0;
    return ret;
}

static inline int atoi(const char* str) {
    int result = 0;
    int sign = 1;
    int i = 0;

    if (str[0] == '-') {
        sign = -1;
        i++;
    } else if (str[0] == '+') {
        i++;
    }

    for (; str[i] != '\0'; i++) {
        result = result * 10 + str[i] - '0';
    }
    return sign * result;
}

#endif /* SHELL_STRING_UTILS_H */