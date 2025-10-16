#ifndef ACE_MACROS_H
#define ACE_MACROS_H

#define ALIGN(value, alignment) \
    (((value) + ((alignment) - 1)) & ~((alignment) - 1))

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#endif /* ACE_MACROS_H */
