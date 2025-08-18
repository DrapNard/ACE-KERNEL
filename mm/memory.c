#include "../include/memory.h"
#include "../include/vga.h"
#include <stdio.h>

// Gestionnaire de mémoire global
static struct memory_manager mem_manager;
static u8 heap_memory[HEAP_SIZE];
static int memory_initialized = 0;

// Initialiser le gestionnaire de mémoire
void memory_init(void) {
    // Initialiser le premier bloc
    struct memory_block* first_block = (struct memory_block*)heap_memory;
    first_block->size = HEAP_SIZE - sizeof(struct memory_block);
    first_block->used = 0;
    first_block->next = NULL;
    first_block->prev = NULL;
    
    // Initialiser le gestionnaire
    mem_manager.first_block = first_block;
    mem_manager.total_memory = HEAP_SIZE;
    mem_manager.used_memory = sizeof(struct memory_block);
    mem_manager.free_memory = HEAP_SIZE - sizeof(struct memory_block);
    
    memory_initialized = 1;
}

// Allouer de la mémoire
void* kmalloc(u32 size) {
    if (!memory_initialized || size == 0) {
        return NULL;
    }
    
    // Aligner la taille sur 4 bytes
    size = ALIGN(size, 4);
    
    struct memory_block* current = mem_manager.first_block;
    
    // Chercher un bloc libre suffisamment grand
    while (current) {
        if (!current->used && current->size >= size) {
            // Si le bloc est plus grand que nécessaire, le diviser
            if (current->size > size + sizeof(struct memory_block) + 4) {
                struct memory_block* new_block = (struct memory_block*)((u8*)current + sizeof(struct memory_block) + size);
                new_block->size = current->size - size - sizeof(struct memory_block);
                new_block->used = 0;
                new_block->next = current->next;
                new_block->prev = current;
                
                if (current->next) {
                    current->next->prev = new_block;
                }
                current->next = new_block;
                current->size = size;
            }
            
            current->used = 1;
            mem_manager.used_memory += current->size;
            mem_manager.free_memory -= current->size;
            
            return (void*)((u8*)current + sizeof(struct memory_block));
        }
        current = current->next;
    }
    
    return NULL; // Pas assez de mémoire
}

// Libérer de la mémoire
void kfree(void* ptr) {
    if (!ptr || !memory_initialized) {
        return;
    }
    
    struct memory_block* block = (struct memory_block*)((u8*)ptr - sizeof(struct memory_block));
    
    if (!block->used) {
        return; // Déjà libéré
    }
    
    block->used = 0;
    mem_manager.used_memory -= block->size;
    mem_manager.free_memory += block->size;
    
    // Fusionner avec le bloc suivant s'il est libre
    if (block->next && !block->next->used) {
        block->size += block->next->size + sizeof(struct memory_block);
        if (block->next->next) {
            block->next->next->prev = block;
        }
        block->next = block->next->next;
    }
    
    // Fusionner avec le bloc précédent s'il est libre
    if (block->prev && !block->prev->used) {
        block->prev->size += block->size + sizeof(struct memory_block);
        if (block->next) {
            block->next->prev = block->prev;
        }
        block->prev->next = block->next;
    }
}

// Réallouer de la mémoire
void* krealloc(void* ptr, u32 new_size) {
    if (!ptr) {
        return kmalloc(new_size);
    }
    
    if (new_size == 0) {
        kfree(ptr);
        return NULL;
    }
    
    void* new_ptr = kmalloc(new_size);
    if (!new_ptr) {
        return NULL;
    }
    
    struct memory_block* old_block = (struct memory_block*)((u8*)ptr - sizeof(struct memory_block));
    u32 copy_size = (old_block->size < new_size) ? old_block->size : new_size;
    
    memcpy(new_ptr, ptr, copy_size);
    kfree(ptr);
    
    return new_ptr;
}

// Afficher les informations mémoire
void memory_info(void) {
    printf("=== Informations Memoire ===\n");
    printf("Memoire totale: %u bytes\n", mem_manager.total_memory);
    printf("Memoire utilisee: %u bytes\n", mem_manager.used_memory);
    printf("Memoire libre: %u bytes\n", mem_manager.free_memory);
}

// Fonctions utilitaires
void* memset(void* dest, int val, u32 count) {
    u8* ptr = (u8*)dest;
    while (count--) {
        *ptr++ = (u8)val;
    }
    return dest;
}

void* memcpy(void* dest, const void* src, u32 count) {
    u8* d = (u8*)dest;
    const u8* s = (const u8*)src;
    while (count--) {
        *d++ = *s++;
    }
    return dest;
}

int memcmp(const void* ptr1, const void* ptr2, u32 count) {
    const u8* p1 = (const u8*)ptr1;
    const u8* p2 = (const u8*)ptr2;
    while (count--) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    return 0;
}