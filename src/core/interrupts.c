#include "core/kernel.h"
#include "core/interrupts.h"
#include "core/scheduler.h"
#include "drivers/vga.h"
#include "arch/x86/io.h"
#include "drivers/keyboard.h"  // Add this for keyboard_interrupt_handler

// Table des descripteurs d'interruption
static struct idt_entry idt[IDT_SIZE];
static struct idt_ptr idt_pointer;

// Gestionnaires d'interruptions
static interrupt_handler_t interrupt_handlers[IDT_SIZE];

// Compteur de ticks du timer
static u32 timer_ticks = 0;

// Configurer une entrée IDT
static void set_idt_entry(u8 num, u32 base, u16 sel, u8 flags) {
    // Since u8 can only hold 0-255, the check is redundant but kept for safety
    if (num >= IDT_SIZE) {
        kernel_panic("Invalid IDT entry number");
        return;
    }
    
    idt[num].offset_low = base & 0xFFFF;
    idt[num].offset_high = (base >> 16) & 0xFFFF;
    idt[num].selector = sel;
    idt[num].zero = 0;
    idt[num].type_attr = flags;
}

// Initialiser les interruptions
void interrupts_init(void) {
    // Initialiser tous les gestionnaires à NULL
    for (int i = 0; i < IDT_SIZE; i++) {
        interrupt_handlers[i] = NULL;
    }
    
    // Remapper les PICs
    pic_remap(0x20, 0x28);  // Remap to 32-47
    
    // Configurer l'IDT avec les stubs d'assemblage
    set_idt_entry(0, (u32)isr0, 0x08, 0x8E);   // Divide by zero
    set_idt_entry(1, (u32)isr1, 0x08, 0x8E);   // Debug
    set_idt_entry(2, (u32)isr2, 0x08, 0x8E);   // NMI
    set_idt_entry(3, (u32)isr3, 0x08, 0x8E);   // Breakpoint
    set_idt_entry(4, (u32)isr4, 0x08, 0x8E);   // Overflow
    set_idt_entry(5, (u32)isr5, 0x08, 0x8E);   // Bounds
    set_idt_entry(6, (u32)isr6, 0x08, 0x8E);   // Invalid opcode
    set_idt_entry(7, (u32)isr7, 0x08, 0x8E);   // Device not available
    set_idt_entry(8, (u32)isr8, 0x08, 0x8E);   // Double fault
    set_idt_entry(9, (u32)isr9, 0x08, 0x8E);   // Coprocessor segment overrun
    set_idt_entry(10, (u32)isr10, 0x08, 0x8E); // Invalid TSS
    set_idt_entry(11, (u32)isr11, 0x08, 0x8E); // Segment not present
    set_idt_entry(12, (u32)isr12, 0x08, 0x8E); // Stack segment fault
    set_idt_entry(13, (u32)isr13, 0x08, 0x8E); // General protection fault
    set_idt_entry(14, (u32)isr14, 0x08, 0x8E); // Page fault
    set_idt_entry(16, (u32)isr16, 0x08, 0x8E); // x87 floating point exception
    set_idt_entry(17, (u32)isr17, 0x08, 0x8E); // Alignment check
    set_idt_entry(18, (u32)isr18, 0x08, 0x8E); // Machine check
    set_idt_entry(19, (u32)isr19, 0x08, 0x8E); // SIMD floating point exception

    // IRQ handlers
    set_idt_entry(32, (u32)irq0, 0x08, 0x8E);  // Timer
    set_idt_entry(33, (u32)irq1, 0x08, 0x8E);  // Keyboard
    set_idt_entry(34, (u32)irq2, 0x08, 0x8E);  // Cascade
    set_idt_entry(35, (u32)irq3, 0x08, 0x8E);  // COM2
    set_idt_entry(36, (u32)irq4, 0x08, 0x8E);  // COM1
    set_idt_entry(37, (u32)irq5, 0x08, 0x8E);  // LPT2
    set_idt_entry(38, (u32)irq6, 0x08, 0x8E);  // Floppy disk
    set_idt_entry(39, (u32)irq7, 0x08, 0x8E);  // LPT1
    set_idt_entry(40, (u32)irq8, 0x08, 0x8E);  // RTC
    set_idt_entry(41, (u32)irq9, 0x08, 0x8E);  // Free
    set_idt_entry(42, (u32)irq10, 0x08, 0x8E); // Free
    set_idt_entry(43, (u32)irq11, 0x08, 0x8E); // Free
    set_idt_entry(44, (u32)irq12, 0x08, 0x8E); // PS2 Mouse
    set_idt_entry(45, (u32)irq13, 0x08, 0x8E); // FPU
    set_idt_entry(46, (u32)irq14, 0x08, 0x8E); // Primary IDE
    set_idt_entry(47, (u32)irq15, 0x08, 0x8E); // Secondary IDE

    // Configurer des interruptions spécifiques
    set_interrupt_handler(32, timer_interrupt_handler);
    set_interrupt_handler(33, keyboard_interrupt_handler);  // Make sure this exists in keyboard.c
    
    // Configurer le pointeur IDT
    idt_pointer.limit = sizeof(idt) - 1;
    idt_pointer.base = (u32)&idt;
    
    // Charger l'IDT
    load_idt(&idt_pointer);
}

// Définir un gestionnaire d'interruption
void set_interrupt_handler(u8 interrupt, interrupt_handler_t handler) {
    // u8 can only be 0-255, so this check is redundant but kept for clarity
    if (interrupt >= IDT_SIZE) {
        kernel_panic("Invalid interrupt number");
        return;
    }
    interrupt_handlers[interrupt] = handler;
}

// Activer les interruptions
void enable_interrupts(void) {
    enable_interrupts_asm();
}

// Désactiver les interruptions
void disable_interrupts(void) {
    disable_interrupts_asm();
}

// Gestionnaire d'interruption par défaut
void default_interrupt_handler(struct interrupt_frame* frame) {
    const u32 vector = frame ? frame->int_no : 0xFFFFFFFFu;

    vga_print("Interruption recue: ");

    if (!frame) {
        vga_print("frame inconnue\n");
        return;
    }

    if (vector < 32) {
        vga_print("Exception critique\n");
        kernel_panic("Unhandled CPU exception");
    } else {
        vga_print("Interruption non geree\n");
    }

    if (vector >= 32 && vector < 48) {
        pic_send_eoi(vector - 32);
    }
}

// Gestionnaire d'interruption du timer
void timer_interrupt_handler(struct interrupt_frame* frame) {
    (void)frame;

    if (++timer_ticks % 10 == 0) {
        scheduler_run();
    }

    pic_send_eoi(0);  // IRQ0 = timer
}