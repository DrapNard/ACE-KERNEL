#include "../include/interrupts.h"
#include "../include/vga.h"
#include "../include/scheduler.h"

// Table des descripteurs d'interruption
static struct idt_entry idt[IDT_SIZE];
static struct idt_ptr idt_pointer;

// Gestionnaires d'interruptions
static interrupt_handler_t interrupt_handlers[IDT_SIZE];

// Compteur de ticks du timer
static u32 timer_ticks = 0;

// Fonctions pour les ports I/O (version simplifiée)
static inline void outb(u16 port, u8 value) {
    (void)port; (void)value; // Éviter les warnings
    // Dans un vrai kernel, on utiliserait l'assembleur inline
    // asm volatile("outb %0, %1" :: "a"(value), "Nd"(port));
}

static inline u8 inb(u16 port) {
    (void)port; // Éviter les warnings
    // Dans un vrai kernel, on utiliserait l'assembleur inline
    // u8 result;
    // asm volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return 0; // Valeur par défaut pour la compilation
}

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
    
    // Activer les interruptions
    enable_interrupts();
}

// Définir un gestionnaire d'interruption
void set_interrupt_handler(u8 interrupt, interrupt_handler_t handler) {
    // u8 est toujours < 256, donc pas besoin de vérifier
    interrupt_handlers[interrupt] = handler;
}

// Activer les interruptions
void enable_interrupts(void) {
    // Dans un vrai kernel x86: asm volatile("sti");
    // Version simplifiée pour compilation
}

// Désactiver les interruptions
void disable_interrupts(void) {
    // Dans un vrai kernel x86: asm volatile("cli");
    // Version simplifiée pour compilation
}

// Gestionnaire d'interruption par défaut
void default_interrupt_handler(struct interrupt_frame* frame) {
    if (frame->int_no < IDT_SIZE && interrupt_handlers[frame->int_no]) {
        interrupt_handlers[frame->int_no](frame);
    } else {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_print("Interruption non gérée: ");
        // Dans un vrai kernel, on afficherait le numéro
        vga_print("\n");
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    }
    
    // Acquitter l'interruption si c'est une IRQ
    if (frame->int_no >= 32 && frame->int_no < 48) {
        if (frame->int_no >= 40) {
            outb(PIC2_COMMAND, 0x20); // EOI au PIC2
        }
        outb(PIC1_COMMAND, 0x20); // EOI au PIC1
    }
}

// Gestionnaire d'interruption du timer
void timer_interrupt_handler(struct interrupt_frame* frame) {
    (void)frame; // Éviter le warning unused parameter
    timer_ticks++;
    
    // Exécuter l'ordonnanceur tous les 10 ticks
    if (timer_ticks % 10 == 0) {
        scheduler_run();
    }
    
    // Acquitter l'interruption
    outb(PIC1_COMMAND, 0x20);
}

// Gestionnaire d'interruption du clavier
void keyboard_interrupt_handler(struct interrupt_frame* frame) {
    (void)frame; // Éviter le warning unused parameter
    u8 scancode = inb(0x60); // Lire le scancode
    
    // Table de conversion simplifiée (touches principales)
    static char keymap[128] = {
        0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
        '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
    };
    
    // Si c'est une pression de touche (bit 7 = 0)
    if (!(scancode & 0x80) && scancode < 128) {
        char key = keymap[scancode];
        if (key) {
            vga_putchar(key);
        }
    }
    
    // Acquitter l'interruption
    outb(PIC1_COMMAND, 0x20);
}

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