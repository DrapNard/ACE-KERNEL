#ifndef DRIVERS_VGA_H
#define DRIVERS_VGA_H

#include "ace/types.h"

// ──────────────────────────────
// VGA Text Mode Definitions
// ──────────────────────────────

// VGA color palette
enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};

// VGA screen size and memory
#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

// Character structure (matches VGA memory layout)
struct vga_char {
    u8 character;
    u8 color;
} __attribute__((packed));

// ──────────────────────────────
// Function Prototypes
// ──────────────────────────────
void vga_init(void);
void vga_clear(void);
void vga_putchar(char c);
void vga_print(const char* str);
void vga_puts_at(u16 x, u16 y, const char* str);
void vga_printf(const char* fmt, ...);
void vga_set_color(enum vga_color fg, enum vga_color bg);
void vga_set_cursor(u16 x, u16 y);
void vga_scroll(void);
void vga_update_hw_cursor(void);

// ──────────────────────────────
// Utility Color Macros
// ──────────────────────────────
#define VGA_COLOR_INFO()    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK)
#define VGA_COLOR_WARN()    vga_set_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK)
#define VGA_COLOR_ERROR()   vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK)
#define VGA_COLOR_OK()      vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK)
#define VGA_COLOR_DEFAULT() vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK)

#endif /* DRIVERS_VGA_H */
