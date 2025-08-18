#ifndef KERNEL_H
#define KERNEL_H

// Types de base pour le kernel
#ifndef __KERNEL_TYPES_DEFINED__
#define __KERNEL_TYPES_DEFINED__

#ifdef __APPLE__
// Sur macOS, utiliser les types standards
#include <stdint.h>
#include <stddef.h>
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
#else
// Pour un vrai kernel x86, définir manuellement
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;
typedef u32 size_t;
#endif

#endif // __KERNEL_TYPES_DEFINED__

// Constantes du kernel
#define KERNEL_VERSION "1.0"
#define KERNEL_NAME "ACE Micro-Kernel"

// Fonctions principales du kernel
void kernel_main(void);
void kernel_panic(const char* message);

// Macros utiles
#define NULL ((void*)0)
#define TRUE 1
#define FALSE 0

// Alignement mémoire
#define ALIGN(x, a) (((x) + (a) - 1) & ~((a) - 1))

#endif // KERNEL_H