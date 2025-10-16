#ifndef KEYBOARD_H
#define KEYBOARD_H

// Types de base
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

// Fonctions du driver clavier
void keyboard_init(void);
char keyboard_getchar(void);
int keyboard_available(void);
char keyboard_getchar_blocking(void);
int keyboard_readline(char* buffer, int max_length);
void keyboard_flush(void);
struct interrupt_frame;
void keyboard_interrupt_handler(struct interrupt_frame* frame);

#endif // KEYBOARD_H