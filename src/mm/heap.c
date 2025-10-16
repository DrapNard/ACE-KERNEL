#include "core/kernel.h"
#include "drivers/vga.h"
#include "libk/mem.h"
#include "mm/heap.h"


// Gestionnaire de mémoire global
static struct memory_manager mem_manager;
static u8 heap_memory[KERNEL_HEAP_SIZE];
static int memory_initialized = 0;

// Initialiser le gestionnaire de mémoire
void memory_init(void) {
    // Initialiser le premier bloc
    struct memory_block* first_block = (struct memory_block*)heap_memory;
    first_block->size = KERNEL_HEAP_SIZE - sizeof(struct memory_block);
    first_block->used = 0;
    first_block->next = NULL;
    first_block->prev = NULL;
    
    // Initialiser le gestionnaire
    mem_manager.first_block = first_block;
    mem_manager.total_memory = KERNEL_HEAP_SIZE;
    mem_manager.used_memory = sizeof(struct memory_block);
    mem_manager.free_memory = KERNEL_HEAP_SIZE - sizeof(struct memory_block);
    
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
    
    k_memcpy(new_ptr, ptr, copy_size);
    kfree(ptr);
    
    return new_ptr;
}

// Afficher les informations mémoire
void memory_info(void) {
    vga_printf("=== Informations Memoire ===\n");
    vga_printf("Memoire totale: %u bytes\n", mem_manager.total_memory);
    vga_printf("Memoire utilisee: %u bytes\n", mem_manager.used_memory);
    vga_printf("Memoire libre: %u bytes\n", mem_manager.free_memory);
}

// Fonctions utilitaires
/* low-level memory helpers moved to libk/mem.c */
