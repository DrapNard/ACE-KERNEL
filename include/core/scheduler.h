#ifndef CORE_SCHEDULER_H
#define CORE_SCHEDULER_H

#include "ace/types.h"

// États des processus
enum process_state {
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_BLOCKED,
    PROCESS_TERMINATED
};

// Structure d'un processus
struct process {
    u32 pid;                    // ID du processus
    enum process_state state;   // État du processus
    u32 priority;              // Priorité (0 = haute, 255 = basse)
    u32 time_slice;            // Tranche de temps allouée
    u32 time_used;             // Temps utilisé
    
    // Contexte du processus
    struct {
        u32 eax, ebx, ecx, edx;
        u32 esi, edi, esp, ebp;
        u32 eip, eflags;
        u16 cs, ds, es, fs, gs, ss;
    } context;

    void* stack_base;
    
    struct process* next;       // Processus suivant dans la liste
};

// Ordonnanceur
struct scheduler {
    struct process* current_process;
    struct process* process_list;
    u32 process_count;
    u32 next_pid;
};

// Fonctions de l'ordonnanceur
void scheduler_init(void);
void scheduler_run(void);
struct process* create_process(void (*entry_point)(void), u32 priority);
void terminate_process(u32 pid);
void yield(void);
void schedule(void);

#endif /* CORE_SCHEDULER_H */
