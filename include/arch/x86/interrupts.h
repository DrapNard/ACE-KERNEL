#ifndef _ARCH_X86_INTERRUPTS_H
#define _ARCH_X86_INTERRUPTS_H

#include "ace/types.h"

// IDT structure
struct idt_entry {
    u16 offset_low;     // Offset bits 0-15
    u16 selector;       // Code segment selector
    u8 zero;            // Always zero
    u8 type_attr;       // Type and attributes
    u16 offset_high;    // Offset bits 16-31
} __attribute__((packed));

struct idt_ptr {
    u16 limit;          // Size of IDT - 1
    u32 base;           // Base address of IDT
} __attribute__((packed));

// IDT size
#define IDT_SIZE 256

// Structure for interrupt frame
struct interrupt_frame {
    u32 gs, fs, es, ds;          // Pushed by pusha
    u32 edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha
    u32 int_no, err_code;        // Our push
    u32 eip, cs, eflags, useresp, ss; // Pushed by CPU
};

// Assembly functions - forward declarations
extern void load_idt(struct idt_ptr* idt_ptr);
extern void enable_interrupts_asm(void);
extern void disable_interrupts_asm(void);

// ISR stubs (defined in assembly)
extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);

// IRQ stubs (defined in assembly)
extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

// PIC functions
extern void pic_remap(u8 offset1, u8 offset2);
extern void pic_disable(void);
extern void pic_send_eoi(u8 irq);

#endif