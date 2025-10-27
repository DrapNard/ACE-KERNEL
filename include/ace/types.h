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

/* Size types */
typedef u32 size_t;
typedef s32 ssize_t;

/* Pointer types */
typedef u32 uintptr_t;
typedef s32 intptr_t;

/* Boolean type */
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

/* Security and utility types */

/* Memory protection attributes */
typedef enum {
    MEM_READ    = 0x01,
    MEM_WRITE   = 0x02,
    MEM_EXEC    = 0x04,
    MEM_USER    = 0x08,
    MEM_PRESENT = 0x10,
    MEM_CACHED  = 0x20,
    MEM_UNCACHED = 0x40,
} mem_flags_t;

/* Interrupt priority levels */
typedef enum {
    IPL_LOWEST  = 0,    /* Background tasks */
    IPL_NORMAL  = 1,    /* Normal interrupts */
    IPL_HIGH    = 2,    /* High priority interrupts */
    IPL_CRITICAL = 3,   /* Critical sections */
} ipl_t;

/* Error codes */
typedef enum {
    ERR_SUCCESS = 0,
    ERR_INVALID_PARAM = -1,
    ERR_OUT_OF_MEMORY = -2,
    ERR_NOT_FOUND = -3,
    ERR_PERMISSION_DENIED = -4,
    ERR_DEVICE_ERROR = -5,
    ERR_TIMEOUT = -6,
    ERR_OVERFLOW = -7,
    ERR_UNDERFLOW = -8,
    ERR_NOT_IMPLEMENTED = -9,
    ERR_ALREADY_EXISTS = -10,
    ERR_BUSY = -11,
    ERR_NO_RESOURCES = -12,
    ERR_ACCESS_VIOLATION = -13,
} error_t;

/* Status codes */
typedef enum {
    STATUS_OK = 0,
    STATUS_ERROR = 1,
    STATUS_PENDING = 2,
    STATUS_ABORTED = 3,
    STATUS_TIMEOUT = 4,
    STATUS_CANCELLED = 5,
} status_t;

/* Memory page sizes */
typedef enum {
    PAGE_SIZE_4KB  = 0x1000,      /* 4KB page */
    PAGE_SIZE_2MB  = 0x200000,    /* 2MB page */
    PAGE_SIZE_1GB  = 0x40000000,  /* 1GB page */
} page_size_t;

/* Security levels */
typedef enum {
    SEC_LEVEL_UNTRUSTED = 0,    /* Untrusted user code */
    SEC_LEVEL_USER      = 1,    /* User space */
    SEC_LEVEL_DRIVER    = 2,    /* Driver code */
    SEC_LEVEL_KERNEL    = 3,    /* Kernel space */
    SEC_LEVEL_HYPERVISOR = 4,   /* Hypervisor level */
} security_level_t;

/* CPU privilege levels */
typedef enum {
    CPL_RING0 = 0,  /* Kernel mode */
    CPL_RING1 = 1,  /* Ring 1 */
    CPL_RING2 = 2,  /* Ring 2 */
    CPL_RING3 = 3,  /* User mode */
} cpl_t;

/* Alignment types for memory */
#define ALIGN_DOWN(addr, align) ((addr) & ~((align) - 1))
#define ALIGN_UP(addr, align)   (((addr) + (align) - 1) & ~((align) - 1))
#define IS_ALIGNED(addr, align) (((addr) & ((align) - 1)) == 0)

/* Safe macros for array operations */
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/* Atomic operation types */
typedef volatile u32 atomic_t;
typedef volatile u64 atomic64_t;

/* Priority levels for scheduling */
typedef enum {
    PRIO_IDLE     = 0,    /* Idle task */
    PRIO_LOW      = 1,    /* Low priority */
    PRIO_NORMAL   = 2,    /* Normal priority */
    PRIO_HIGH     = 3,    /* High priority */
    PRIO_REALTIME = 4,    /* Real-time priority */
    PRIO_CRITICAL = 5,    /* Critical priority */
} priority_t;

/* File system related types */
typedef u32 inode_t;
typedef u64 block_t;
typedef u32 sector_t;

/* Time types */
typedef u64 timestamp_t;      /* Timestamp in nanoseconds */
typedef u32 duration_t;       /* Duration in milliseconds */
typedef u32 tick_t;           /* System ticks */

/* Process and thread IDs */
typedef u32 pid_t;
typedef u32 tid_t;

/* Security context */
typedef struct {
    security_level_t level;
    u32 permissions;
    u32 user_id;
    u32 group_id;
} security_context_t;

/* Memory region descriptor */
typedef struct {
    uintptr_t start;
    size_t size;
    mem_flags_t flags;
    security_level_t security_level;
} mem_region_t;

/* Safe type checking macros */
#define TYPE_CHECK(var, type) __builtin_types_compatible_p(__typeof__(var), type)

/* Bounds checking for arrays */
#define BOUNDS_CHECK(arr, index, size) ((index) < (size))

/* Compile-time assertions */
#define STATIC_ASSERT(condition, message) \
    typedef char static_assert_##message[(condition) ? 1 : -1]

/* Verify basic types */
STATIC_ASSERT(sizeof(u8) == 1, u8_must_be_1_byte);
STATIC_ASSERT(sizeof(u16) == 2, u16_must_be_2_bytes);
STATIC_ASSERT(sizeof(u32) == 4, u32_must_be_4_bytes);
STATIC_ASSERT(sizeof(u64) == 8, u64_must_be_8_bytes);

#endif /* ACE_TYPES_H */