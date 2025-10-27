#ifndef _CORE_INTERRUPTS_H
#define _CORE_INTERRUPTS_H

#include "ace/types.h"
#include "arch/x86/interrupts.h"

typedef void (*interrupt_handler_t)(struct interrupt_frame* frame);

// Function prototypes
void interrupts_init(void);
void set_interrupt_handler(u8 interrupt, interrupt_handler_t handler);
void enable_interrupts(void);
void disable_interrupts(void);
void default_interrupt_handler(struct interrupt_frame* frame);
void timer_interrupt_handler(struct interrupt_frame* frame);

#endif