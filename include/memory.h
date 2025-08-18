#ifndef MEMORY_H
#define MEMORY_H

#include "kernel.h"

// Constantes mémoire
#define PAGE_SIZE 4096
#define HEAP_START 0x100000  // 1MB
#define HEAP_SIZE 0x100000   // 1MB heap

// Structure d'un bloc mémoire
struct memory_block {
    u32 size;
    u32 used;
    struct memory_block* next;
    struct memory_block* prev;
};

// Gestionnaire de mémoire
struct memory_manager {
    struct memory_block* first_block;
    u32 total_memory;
    u32 used_memory;
    u32 free_memory;
};

// Fonctions de gestion mémoire
void memory_init(void);
void* kmalloc(u32 size);
void kfree(void* ptr);
void* krealloc(void* ptr, u32 new_size);
void memory_info(void);

// Fonctions utilitaires
void* memset(void* dest, int val, u32 count);
void* memcpy(void* dest, const void* src, u32 count);
int memcmp(const void* ptr1, const void* ptr2, u32 count);

#endif // MEMORY_H