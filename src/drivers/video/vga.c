#include "drivers/vga.h"
#include "libk/mem.h"
#if defined(__i386__) || defined(__x86_64__)
#include "arch/x86/io.h"
#endif
#include "libk/stdarg.h"

static struct vga_char* vga_buffer = (struct vga_char*)VGA_MEMORY;
static u16 vga_row = 0;
static u16 vga_column = 0;
static u8 vga_color = 0;

// ──────────────────────────────
// Internal helpers
// ──────────────────────────────
static inline u8 vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

static inline struct vga_char vga_entry(unsigned char uc, u8 color) {
    return (struct vga_char){ uc, color };
}

// ──────────────────────────────
// Initialization
// ──────────────────────────────
void vga_init(void) {
    vga_row = 0;
    vga_column = 0;
    vga_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_clear();
}

// ──────────────────────────────
// Core VGA functions
// ──────────────────────────────
void vga_clear(void) {
    for (u16 i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
        vga_buffer[i] = vga_entry(' ', vga_color);
    vga_row = vga_column = 0;
    vga_update_hw_cursor();
}

void vga_set_color(enum vga_color fg, enum vga_color bg) {
    vga_color = vga_entry_color(fg, bg);
}

void vga_set_cursor(u16 x, u16 y) {
    if (x < VGA_WIDTH && y < VGA_HEIGHT) {
        vga_column = x;
        vga_row = y;
        vga_update_hw_cursor();
    }
}

void vga_scroll(void) {
    k_memmove(vga_buffer,
              vga_buffer + VGA_WIDTH,
              (VGA_HEIGHT - 1) * VGA_WIDTH * sizeof(struct vga_char));

    for (u16 x = 0; x < VGA_WIDTH; x++)
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', vga_color);

    vga_row = VGA_HEIGHT - 1;
    vga_update_hw_cursor();
}

// ──────────────────────────────
// Character output
// ──────────────────────────────
void vga_putchar(char c) {
    if (c == '\n') {
        vga_column = 0;
        if (++vga_row == VGA_HEIGHT) vga_scroll();
    } else if (c == '\r') {
        vga_column = 0;
    } else if (c == '\t') {
        vga_column = (vga_column + 8) & ~(8 - 1);
        if (vga_column >= VGA_WIDTH) {
            vga_column = 0;
            if (++vga_row == VGA_HEIGHT) vga_scroll();
        }
    } else {
        const u16 index = vga_row * VGA_WIDTH + vga_column;
        vga_buffer[index] = vga_entry(c, vga_color);
        if (++vga_column == VGA_WIDTH) {
            vga_column = 0;
            if (++vga_row == VGA_HEIGHT) vga_scroll();
        }
    }
    vga_update_hw_cursor();
}

void vga_print(const char* str) {
    if (!str) return;
    while (*str) vga_putchar(*str++);
}

void vga_puts_at(u16 x, u16 y, const char* str) {
    if (!str || x >= VGA_WIDTH || y >= VGA_HEIGHT) return;
    u16 index = y * VGA_WIDTH + x;
    while (*str && x < VGA_WIDTH) {
        vga_buffer[index++] = vga_entry(*str++, vga_color);
        x++;
    }
}

// ──────────────────────────────
// Mini printf (no stdlib dependency)
// ──────────────────────────────
static void vga_print_hex(u32 num) {
    char hex[9];
    for (int i = 7; i >= 0; i--) {
        u8 nibble = num & 0xF;
        hex[i] = (nibble < 10) ? ('0' + nibble) : ('A' + nibble - 10);
        num >>= 4;
    }
    hex[8] = '\0';
    vga_print("0x");
    vga_print(hex);
}

static void vga_print_dec(u32 num) {
    char buf[11];
    int i = 10;
    buf[i--] = '\0';
    if (num == 0) {
        vga_putchar('0');
        return;
    }
    while (num && i >= 0) {
        buf[i--] = '0' + (num % 10);
        num /= 10;
    }
    vga_print(&buf[i + 1]);
}

void vga_printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    while (*fmt) {
        if (*fmt == '%') {
            ++fmt;
            switch (*fmt) {
                case 's': {
                    const char* str = va_arg(args, const char*);
                    vga_print(str);
                    break;
                }
                case 'd': {
                    u32 num = va_arg(args, u32);
                    vga_print_dec(num);
                    break;
                }
                case 'x': {
                    u32 num = va_arg(args, u32);
                    vga_print_hex(num);
                    break;
                }
                case 'p': {
                    u32 num = (u32)va_arg(args, void*);
                    vga_print("0x");
                    vga_print_hex(num);
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    vga_putchar(c);
                    break;
                }
                case '%':
                    vga_putchar('%');
                    break;
                default:
                    vga_putchar('%');
                    vga_putchar(*fmt);
                    break;
            }
        } else {
            vga_putchar(*fmt);
        }
        ++fmt;
    }

    va_end(args);
}

// ──────────────────────────────
// Hardware Cursor (x86 only)
// ──────────────────────────────
#if defined(__i386__) || defined(__x86_64__)
#define VGA_CMD_PORT  0x3D4
#define VGA_DATA_PORT 0x3D5

void vga_update_hw_cursor(void) {
    u16 pos = vga_row * VGA_WIDTH + vga_column;
    outb(VGA_CMD_PORT, 0x0F);
    outb(VGA_DATA_PORT, (u8)(pos & 0xFF));
    outb(VGA_CMD_PORT, 0x0E);
    outb(VGA_DATA_PORT, (u8)((pos >> 8) & 0xFF));
}
#else
void vga_update_hw_cursor(void) { /* no-op on RISC-V */ }
#endif
