#ifndef DRIVERS_VGA_H
#define DRIVERS_VGA_H

#include "ace/types.h"
#include "libk/stdarg.h"

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

// VGA context structure for thread safety
typedef struct {
    u16 cursor_x;
    u16 cursor_y;
    u8 color;
    u16 saved_cursor_x;
    u16 saved_cursor_y;
    bool is_cursor_visible;
    bool auto_scroll;
    u8* saved_screen;  // For screen save/restore
    size_t saved_screen_size;
} vga_context_t;

// ──────────────────────────────
// Function Prototypes
// ──────────────────────────────

// Basic operations
void vga_init(void);
void vga_clear(void);
void vga_clear_region(u16 x, u16 y, u16 width, u16 height);
void vga_putchar(char c);
void vga_putchar_at(u16 x, u16 y, char c);
void vga_print(const char* str);
void vga_print_at(u16 x, u16 y, const char* str);
void vga_print_char(u16 x, u16 y, char c, u8 color);
void vga_printf(const char* fmt, ...);
void vga_vprintf(const char* fmt, va_list args);

// Color and cursor operations
void vga_set_color(enum vga_color fg, enum vga_color bg);
void vga_set_cursor(u16 x, u16 y);
void vga_hide_cursor(void);
void vga_show_cursor(void);
void vga_save_cursor(void);
void vga_restore_cursor(void);
void vga_scroll(void);
void vga_update_hw_cursor(void);

// Advanced text operations
void vga_draw_box(u16 x, u16 y, u16 width, u16 height, u8 color);
void vga_draw_line(u16 x1, u16 y1, u16 x2, u16 y2, char c, u8 color);
void vga_fill_rect(u16 x, u16 y, u16 width, u16 height, char c, u8 color);
void vga_invert_rect(u16 x, u16 y, u16 width, u16 height);

// Screen management
void vga_save_screen(void);
void vga_restore_screen(void);
void vga_swap_buffers(void);
void vga_set_auto_scroll(bool enable);

// Utility functions
u16 vga_get_cursor_x(void);
u16 vga_get_cursor_y(void);
u8 vga_get_current_color(void);
u16 vga_string_width(const char* str);
void vga_center_text(u16 y, const char* str);
void vga_right_align(u16 y, const char* str);
void vga_left_align(u16 y, const char* str);

// Context management (for multi-threading support)
vga_context_t* vga_create_context(void);
void vga_destroy_context(vga_context_t* ctx);
void vga_use_context(vga_context_t* ctx);
void vga_reset_context(void);

// ──────────────────────────────
// Utility Color Macros
// ──────────────────────────────
#define VGA_COLOR_INFO()    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK)
#define VGA_COLOR_WARN()    vga_set_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK)
#define VGA_COLOR_ERROR()   vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK)
#define VGA_COLOR_OK()      vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK)
#define VGA_COLOR_DEFAULT() vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK)
#define VGA_COLOR_DEBUG()   vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK)

// ──────────────────────────────
// Security and Safety Macros
// ──────────────────────────────
#define VGA_BOUNDS_CHECK(x, y) ((x) < VGA_WIDTH && (y) < VGA_HEIGHT)
#define VGA_VALIDATE_COORDS(x, y) do { \
    if (!(x < VGA_WIDTH && y < VGA_HEIGHT)) return; \
} while(0)

#endif /* DRIVERS_VGA_H */