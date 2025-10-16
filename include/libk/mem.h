#ifndef LIBK_MEM_H
#define LIBK_MEM_H

#include "../ace/types.h"

void* k_memcpy(void* dest, const void* src, size_t count);
void* k_memset(void* dest, int value, size_t count);
int   k_memcmp(const void* lhs, const void* rhs, size_t count);
void* k_memmove(void* dest, const void* src, size_t count);

#endif /* LIBK_MEM_H */
