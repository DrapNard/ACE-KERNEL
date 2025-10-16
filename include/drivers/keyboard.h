#ifndef DRIVERS_KEYBOARD_H
#define DRIVERS_KEYBOARD_H

#include "ace/types.h"

struct interrupt_frame;

void keyboard_init(void);
char keyboard_getchar(void);
int  keyboard_available(void);
char keyboard_getchar_blocking(void);
int  keyboard_readline(char* buffer, int max_length);
void keyboard_flush(void);
void keyboard_interrupt_handler(struct interrupt_frame* frame);

#endif /* DRIVERS_KEYBOARD_H */
