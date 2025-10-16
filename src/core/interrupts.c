#include "core/kernel.h"
#include "core/interrupts.h"
#include "core/scheduler.h"
#include "drivers/vga.h"
#include "arch/x86/io.h"

// Table des descripteurs d'interruption
static struct idt_entry idt[IDT_SIZE];
static struct idt_ptr idt_pointer;

// Gestionnaires d'interruptions
static interrupt_handler_t interrupt_handlers[IDT_SIZE];

// Compteur de ticks du timer
static u32 timer_ticks = 0;

// Configurer une entrée IDT
static void set_idt_entry(u8 num, u32 base, u16 sel, u8 flags) {
    idt[num].offset_low = base & 0xFFFF;
    idt[num].offset_high = (base >> 16) & 0xFFFF;
    idt[num].selector = sel;
    idt[num].zero = 0;
    idt[num].type_attr = flags;
}

// Remapper les PICs
static void remap_pics(void) {
    // Sauvegarder les masques
    u8 mask1 = inb(PIC1_DATA);
    u8 mask2 = inb(PIC2_DATA);
    
    // Initialiser les PICs
    outb(PIC1_COMMAND, 0x11); // Commande d'initialisation
    outb(PIC2_COMMAND, 0x11);
    
    // Définir les offsets des vecteurs d'interruption
    outb(PIC1_DATA, 0x20); // PIC1 commence à 0x20 (32)
    outb(PIC2_DATA, 0x28); // PIC2 commence à 0x28 (40)
    
    // Configuration en cascade
    outb(PIC1_DATA, 0x04); // PIC1 a un esclave sur IRQ2
    outb(PIC2_DATA, 0x02); // PIC2 est l'esclave
    
    // Mode 8086
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);
    
    // Restaurer les masques
    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);
}

// Initialiser les interruptions
void interrupts_init(void) {
    // Initialiser tous les gestionnaires à NULL
    for (int i = 0; i < IDT_SIZE; i++) {
        interrupt_handlers[i] = NULL;
    }
    
    // Remapper les PICs
    remap_pics();
    
    // Configurer l'IDT (version simplifiée)
    // Dans un vrai kernel, on aurait des stubs assembleur pour chaque interruption
    for (int i = 0; i < IDT_SIZE; i++) {
        set_idt_entry(i, (u32)default_interrupt_handler, 0x08, 0x8E);
    }
    
    // Configurer des interruptions spécifiques
    set_interrupt_handler(32, timer_interrupt_handler);    // Timer (IRQ0)
    set_interrupt_handler(33, keyboard_interrupt_handler); // Clavier (IRQ1)
    
    // Configurer le pointeur IDT
    idt_pointer.limit = sizeof(idt) - 1;
    idt_pointer.base = (u32)&idt;
    
    // Charger l'IDT (version simplifiée pour compilation)
    // Dans un vrai kernel x86, on utiliserait: asm volatile("lidt %0" : : "m"(idt_pointer));
    // Pour l'instant, on simule juste le chargement
    (void)idt_pointer;
    
    // Les interruptions restent désactivées tant que les stubs assembleur ne
    // sont pas en place. Appelez enable_interrupts() explicitement une fois
    // que des gestionnaires bas niveau existent.
}

// Définir un gestionnaire d'interruption
void set_interrupt_handler(u8 interrupt, interrupt_handler_t handler) {
    // u8 est toujours < 256, donc pas besoin de vérifier
    interrupt_handlers[interrupt] = handler;
}

// Activer les interruptions
void enable_interrupts(void) {
    // Dans un vrai kernel x86: 
    asm volatile("sti");
    // Version simplifiée pour compilation
}

// Désactiver les interruptions
void disable_interrupts(void) {
    // Dans un vrai kernel x86: 
    asm volatile("cli");
    // Version simplifiée pour compilation
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
        if (vector >= 40) {
            outb(PIC2_COMMAND, 0x20);
        }
        outb(PIC1_COMMAND, 0x20);
    }
}

// Gestionnaire d'interruption du timer
void timer_interrupt_handler(struct interrupt_frame* frame) {
    (void)frame;

    if (++timer_ticks % 10 == 0) {
        scheduler_run();
    }

    outb(PIC1_COMMAND, 0x20);
}

// Le gestionnaire clavier est maintenant dans drivers/keyboard.c

// Stubs assembleur simplifiés (normalement dans un fichier .s séparé)
// Ces fonctions devraient être implémentées en assembleur
void load_idt(struct idt_ptr* idt_ptr) {
    // Dans un vrai kernel x86: asm volatile("lidt %0" : : "m"(*idt_ptr));
    (void)idt_ptr; // Version simplifiée pour compilation
}

void isr0(void) {
    // Stub pour ISR 0
}

void isr1(void) {
    // Stub pour ISR 1
}

void irq0(void) {
    // Stub pour IRQ 0 (timer)
}

void irq1(void) {
    // Stub pour IRQ 1 (clavier)
}
