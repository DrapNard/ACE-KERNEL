#include "../include/scheduler.h"
#include "../include/memory.h"
#include "../drivers/vga/vga.h"

static struct scheduler main_scheduler;
static int scheduler_initialized = 0;

static void idle_process(void) {
    while (1) {
        asm volatile("hlt");
        // Dans un vrai kernel x86: asm volatile("hlt");
        // Attendre une interruption - version simplifiée pour compilation
    }
}

void scheduler_init(void) {
    main_scheduler.current_process = NULL;
    main_scheduler.process_list = NULL;
    main_scheduler.process_count = 0;
    main_scheduler.next_pid = 1;

    create_process(idle_process, 255); // Priorité la plus basse

    scheduler_initialized = 1;
}

struct process* create_process(void (*entry_point)(void), u32 priority) {
    if (!scheduler_initialized) {
        return NULL;
    }

    struct process* new_process = (struct process*)kmalloc(sizeof(struct process));
    if (!new_process) {
        return NULL;
    }

    new_process->pid = main_scheduler.next_pid++;
    new_process->state = PROCESS_READY;
    new_process->priority = priority;
    new_process->time_slice = 10;
    new_process->time_used = 0;

    memset(&new_process->context, 0, sizeof(new_process->context));
    new_process->context.eip = (u32)entry_point;
    new_process->context.eflags = 0x202;
    new_process->context.cs = 0x08;
    new_process->context.ds = 0x10;
    new_process->context.es = 0x10;
    new_process->context.fs = 0x10;
    new_process->context.gs = 0x10;
    new_process->context.ss = 0x10;

    void* stack = kmalloc(4096);
    if (!stack) {
        kfree(new_process);
        return NULL;
    }
    new_process->context.esp = (u32)stack + 4096 - 4;
    new_process->context.ebp = new_process->context.esp;

    new_process->next = main_scheduler.process_list;
    main_scheduler.process_list = new_process;
    main_scheduler.process_count++;

    if (!main_scheduler.current_process) {
        main_scheduler.current_process = new_process;
        new_process->state = PROCESS_RUNNING;
    }

    return new_process;
}

void terminate_process(u32 pid) {
    struct process* current = main_scheduler.process_list;
    struct process* prev = NULL;

    while (current) {
        if (current->pid == pid) {
            current->state = PROCESS_TERMINATED;

            if (prev) {
                prev->next = current->next;
            } else {
                main_scheduler.process_list = current->next;
            }

            if (main_scheduler.current_process == current) {
                main_scheduler.current_process = NULL;
                schedule();
            }

            kfree((void*)(current->context.esp - 4092)); // Pile
            kfree(current); // Structure
            main_scheduler.process_count--;

            return;
        }
        prev = current;
        current = current->next;
    }
}

void yield(void) {
    if (main_scheduler.current_process) {
        main_scheduler.current_process->state = PROCESS_READY;
    }
    schedule();
}

void schedule(void) {
    if (!scheduler_initialized || !main_scheduler.process_list) {
        return;
    }

    struct process* next_process = NULL;
    struct process* current = main_scheduler.process_list;

    if (!main_scheduler.current_process) {
        while (current) {
            if (current->state == PROCESS_READY) {
                next_process = current;
                break;
            }
            current = current->next;
        }
    } else {
        struct process* start = main_scheduler.current_process->next;
        if (!start) start = main_scheduler.process_list;

        current = start;
        do {
            if (current->state == PROCESS_READY) {
                next_process = current;
                break;
            }
            current = current->next;
            if (!current) current = main_scheduler.process_list;
        } while (current != start);
    }

    if (next_process && next_process != main_scheduler.current_process) {
        if (main_scheduler.current_process) {
            main_scheduler.current_process->state = PROCESS_READY;
        }

        main_scheduler.current_process = next_process;
        next_process->state = PROCESS_RUNNING;
        next_process->time_used = 0;

    }
}

void scheduler_run(void) {
    if (!scheduler_initialized) {
        return;
    }

    if (main_scheduler.current_process &&
        main_scheduler.current_process->state == PROCESS_RUNNING) {
        main_scheduler.current_process->time_used++;

        if (main_scheduler.current_process->time_used >=
            main_scheduler.current_process->time_slice) {
            schedule();
        }
    } else {
        schedule();
    }
}
