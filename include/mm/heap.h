#ifndef MM_HEAP_H
#define MM_HEAP_H

#include "ace/types.h"
#include "ace/macros.h"

#define KERNEL_PAGE_SIZE 4096
#define KERNEL_HEAP_START 0x00100000
#define KERNEL_HEAP_SIZE  0x00100000 /* 1 MiB heap */

struct memory_block {
    u32 size;
    u32 used;
    struct memory_block* next;
    struct memory_block* prev;
};

struct memory_manager {
    struct memory_block* first_block;
    u32 total_memory;
    u32 used_memory;
    u32 free_memory;
};

void memory_init(void);
void* kmalloc(u32 size);
void  kfree(void* ptr);
void* krealloc(void* ptr, u32 new_size);
void  memory_info(void);

#endif /* MM_HEAP_H */
