#include "../include/keyboard.h"
#include "../include/vga.h"
#include "../include/interrupts.h"

// Ports du contrôleur clavier
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

// Buffer circulaire pour les touches
#define KEYBOARD_BUFFER_SIZE 256
static char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
static volatile int buffer_start = 0;
static volatile int buffer_end = 0;
static volatile int buffer_count = 0;

// Table de conversion scancode vers ASCII (layout US)
static char scancode_to_ascii[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*',
    0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// Table pour les touches avec Shift
static char scancode_to_ascii_shift[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,
    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*',
    0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// État des touches modificatrices
static int shift_pressed = 0;
static int ctrl_pressed = 0;
static int alt_pressed = 0;

// Fonctions d'E/S pour les ports (version simplifiée)
static inline u8 inb(u16 port) {
    (void)port; // Éviter les warnings
    // Dans un vrai kernel, on utiliserait l'assembleur inline
    // u8 result;
    // asm volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return 0; // Valeur par défaut pour la compilation
}

static inline void outb(u16 port, u8 data) {
    (void)port; (void)data; // Éviter les warnings
    // Dans un vrai kernel, on utiliserait l'assembleur inline
    // asm volatile("outb %0, %1" :: "a"(data), "Nd"(port));
}

// Gestionnaire d'interruption clavier
void keyboard_interrupt_handler(struct interrupt_frame* frame) {
    u8 scancode = inb(KEYBOARD_DATA_PORT);
    
    // Vérifier si c'est un relâchement de touche (bit 7 = 1)
    int key_released = (scancode & 0x80) != 0;
    scancode &= 0x7F; // Enlever le bit de relâchement
    
    // Gérer les touches modificatrices
    switch (scancode) {
        case 0x2A: // Left Shift
        case 0x36: // Right Shift
            shift_pressed = !key_released;
            return;
        case 0x1D: // Ctrl
            ctrl_pressed = !key_released;
            return;
        case 0x38: // Alt
            alt_pressed = !key_released;
            return;
    }
    
    // Ignorer les relâchements de touches normales
    if (key_released) {
        return;
    }
    
    // Convertir le scancode en ASCII
    char ascii;
    if (shift_pressed) {
        ascii = scancode_to_ascii_shift[scancode];
    } else {
        ascii = scancode_to_ascii[scancode];
    }
    
    // Ajouter au buffer si la touche est valide
    if (ascii != 0 && buffer_count < KEYBOARD_BUFFER_SIZE) {
        keyboard_buffer[buffer_end] = ascii;
        buffer_end = (buffer_end + 1) % KEYBOARD_BUFFER_SIZE;
        buffer_count++;
    }
    
    // Acquitter l'interruption
    outb(0x20, 0x20);
}

// Initialiser le driver clavier
void keyboard_init() {
    buffer_start = 0;
    buffer_end = 0;
    buffer_count = 0;
    
    // Installer le gestionnaire d'interruption pour IRQ1 (clavier)
    set_interrupt_handler(33, keyboard_interrupt_handler); // IRQ1 = INT 33
    
    vga_print("Driver clavier initialise\n");
}

// Lire un caractère du buffer (non-bloquant)
char keyboard_getchar() {
    if (buffer_count == 0) {
        return 0; // Pas de caractère disponible
    }
    
    char c = keyboard_buffer[buffer_start];
    buffer_start = (buffer_start + 1) % KEYBOARD_BUFFER_SIZE;
    buffer_count--;
    
    return c;
}

// Vérifier si des caractères sont disponibles
int keyboard_available() {
    return buffer_count > 0;
}

// Lire un caractère (bloquant)
char keyboard_getchar_blocking() {
    while (!keyboard_available()) {
        // Attendre qu'un caractère soit disponible
        __asm__ volatile ("hlt");
    }
    return keyboard_getchar();
}

// Lire une ligne complète
int keyboard_readline(char* buffer, int max_length) {
    int pos = 0;
    char c;
    
    while (pos < max_length - 1) {
        c = keyboard_getchar_blocking();
        
        if (c == '\n') {
            buffer[pos] = '\0';
            return pos;
        } else if (c == '\b') {
            if (pos > 0) {
                pos--;
                vga_print("\b ");
            }
        } else if (c >= 32 && c <= 126) {
            buffer[pos++] = c;
            vga_putchar(c);
        }
    }
    
    buffer[pos] = '\0';
    return pos;
}

// Vider le buffer clavier
void keyboard_flush() {
    buffer_start = 0;
    buffer_end = 0;
    buffer_count = 0;
}