#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "kernel.h"

#define IDT_SIZE 256
#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1

// Structure d'une entrée IDT
struct idt_entry {
    u16 offset_low;   // Offset bits 0-15
    u16 selector;     // Sélecteur de segment
    u8 zero;          // Toujours 0
    u8 type_attr;     // Type et attributs
    u16 offset_high;  // Offset bits 16-31
} __attribute__((packed));

// Pointeur vers l'IDT
struct idt_ptr {
    u16 limit;
    u32 base;
} __attribute__((packed));

// Structure des registres sauvegardés lors d'une interruption
struct interrupt_frame {
    u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
    u32 int_no, err_code;
    u32 eip, cs, eflags, useresp, ss;
};

// Type de gestionnaire d'interruption
typedef void (*interrupt_handler_t)(struct interrupt_frame* frame);

// Fonctions de gestion des interruptions
void interrupts_init(void);
void set_interrupt_handler(u8 interrupt, interrupt_handler_t handler);
void enable_interrupts(void);
void disable_interrupts(void);

// Gestionnaires d'interruptions par défaut
void default_interrupt_handler(struct interrupt_frame* frame);
void timer_interrupt_handler(struct interrupt_frame* frame);
void keyboard_interrupt_handler(struct interrupt_frame* frame);

// Fonctions assembleur (définies dans interrupts.s)
extern void load_idt(struct idt_ptr* idt_ptr);
extern void isr0(void);
extern void isr1(void);
extern void irq0(void);
extern void irq1(void);

#endif // INTERRUPTS_H
