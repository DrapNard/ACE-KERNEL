#ifndef ACE_TYPES_H
#define ACE_TYPES_H

/* Primitive integer types for the kernel */
typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

typedef signed char        s8;
typedef signed short       s16;
typedef signed int         s32;
typedef signed long long   s64;

typedef u32 size_t;
typedef s32 ssize_t;
typedef u32 uintptr_t;
typedef s32 intptr_t;

typedef u8 bool;

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

#endif /* ACE_TYPES_H */
