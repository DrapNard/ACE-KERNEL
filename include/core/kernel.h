#ifndef CORE_KERNEL_H
#define CORE_KERNEL_H

#include "ace/types.h"
#include "ace/macros.h"

#define KERNEL_VERSION "1.0.0"
#define KERNEL_NAME    "ACE Micro-Kernel"

void kernel_main(void);
void kernel_shutdown(void);
void kernel_panic(const char* message);

#endif /* CORE_KERNEL_H */
