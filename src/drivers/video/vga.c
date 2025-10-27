#include "drivers/vga.h"
#include "libk/mem.h"
#include "mm/heap.h"
#include "libk/stdarg.h"
#if defined(__i386__) || defined(__x86_64__)
#include "arch/x86/io.h"
#endif

// ──────────────────────────────
// Global Variables
// ──────────────────────────────
static struct vga_char* vga_buffer = (struct vga_char*)VGA_MEMORY;
static u16 vga_row = 0;
static u16 vga_column = 0;
static u8 vga_color = 0;
static bool vga_cursor_visible = true;
static bool vga_auto_scroll = true;
static u16 vga_saved_cursor_x = 0;
static u16 vga_saved_cursor_y = 0;

// Context management
static vga_context_t* current_context = NULL;
static vga_context_t default_context = {0};

// ──────────────────────────────
// Internal helpers
// ──────────────────────────────
static inline u8 vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | (bg << 4);
}

static inline struct vga_char vga_entry(unsigned char uc, u8 color) {
    return (struct vga_char){ uc, color };
}

static inline u16 vga_get_index(u16 x, u16 y) {
    return y * VGA_WIDTH + x;
}

// ──────────────────────────────
// Initialization
// ──────────────────────────────
void vga_init(void) {
    vga_row = 0;
    vga_column = 0;
    vga_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_cursor_visible = true;
    vga_auto_scroll = true;
    vga_clear();
    
    // Initialize default context
    default_context.cursor_x = 0;
    default_context.cursor_y = 0;
    default_context.color = vga_color;
    default_context.is_cursor_visible = true;
    default_context.auto_scroll = true;
    current_context = &default_context;
}

// ──────────────────────────────
// Basic VGA functions
// ──────────────────────────────
void vga_clear(void) {
    for (u16 i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
        vga_buffer[i] = vga_entry(' ', vga_color);
    vga_row = vga_column = 0;
    if (current_context) {
        current_context->cursor_x = 0;
        current_context->cursor_y = 0;
    }
    vga_update_hw_cursor();
}

void vga_clear_region(u16 x, u16 y, u16 width, u16 height) {
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) return;
    
    u16 end_x = (x + width > VGA_WIDTH) ? VGA_WIDTH : x + width;
    u16 end_y = (y + height > VGA_HEIGHT) ? VGA_HEIGHT : y + height;
    
    for (u16 cy = y; cy < end_y; cy++) {
        for (u16 cx = x; cx < end_x; cx++) {
            u16 index = vga_get_index(cx, cy);
            vga_buffer[index] = vga_entry(' ', vga_color);
        }
    }
}

void vga_set_color(enum vga_color fg, enum vga_color bg) {
    vga_color = vga_entry_color(fg, bg);
    if (current_context) {
        current_context->color = vga_color;
    }
}

void vga_set_cursor(u16 x, u16 y) {
    if (x < VGA_WIDTH && y < VGA_HEIGHT) {
        vga_column = x;
        vga_row = y;
        if (current_context) {
            current_context->cursor_x = x;
            current_context->cursor_y = y;
        }
        vga_update_hw_cursor();
    }
}

void vga_save_cursor(void) {
    if (current_context) {
        current_context->saved_cursor_x = current_context->cursor_x;
        current_context->saved_cursor_y = current_context->cursor_y;
    }
}

void vga_restore_cursor(void) {
    if (current_context) {
        current_context->cursor_x = current_context->saved_cursor_x;
        current_context->cursor_y = current_context->saved_cursor_y;
        vga_column = current_context->cursor_x;
        vga_row = current_context->cursor_y;
        vga_update_hw_cursor();
    }
}

void vga_hide_cursor(void) {
    vga_cursor_visible = false;
    if (current_context) {
        current_context->is_cursor_visible = false;
    }
    // Note: Hardware cursor visibility is platform-specific
}

void vga_show_cursor(void) {
    vga_cursor_visible = true;
    if (current_context) {
        current_context->is_cursor_visible = true;
    }
    vga_update_hw_cursor();
}

void vga_scroll(void) {
    if (current_context && !current_context->auto_scroll) return;
    
    k_memmove(vga_buffer,
              vga_buffer + VGA_WIDTH,
              (VGA_HEIGHT - 1) * VGA_WIDTH * sizeof(struct vga_char));

    for (u16 x = 0; x < VGA_WIDTH; x++)
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', vga_color);

    vga_row = VGA_HEIGHT - 1;
    if (current_context) {
        current_context->cursor_y = VGA_HEIGHT - 1;
    }
}

void vga_set_auto_scroll(bool enable) {
    if (current_context) {
        current_context->auto_scroll = enable;
    }
}

// ──────────────────────────────
// Character output with bounds checking
// ──────────────────────────────
void vga_putchar_at(u16 x, u16 y, char c) {
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) return;
    
    u16 index = vga_get_index(x, y);
    vga_buffer[index] = vga_entry(c, vga_color);
}

void vga_print_at(u16 x, u16 y, const char* str) {
    if (!str || x >= VGA_WIDTH || y >= VGA_HEIGHT) return;
    
    u16 current_x = x;
    while (*str && current_x < VGA_WIDTH) {
        vga_putchar_at(current_x, y, *str++);
        current_x++;
    }
}

void vga_print_char(u16 x, u16 y, char c, u8 color) {
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) return;
    
    u16 index = vga_get_index(x, y);
    vga_buffer[index] = vga_entry(c, color);
}

void vga_putchar(char c) {
    if (c == '\n') {
        vga_column = 0;
        if (++vga_row == VGA_HEIGHT && vga_auto_scroll) vga_scroll();
    } else if (c == '\r') {
        vga_column = 0;
    } else if (c == '\t') {
        vga_column = (vga_column + 8) & ~(8 - 1);
        if (vga_column >= VGA_WIDTH) {
            vga_column = 0;
            if (++vga_row == VGA_HEIGHT && vga_auto_scroll) vga_scroll();
        }
    } else {
        const u16 index = vga_get_index(vga_column, vga_row);
        vga_buffer[index] = vga_entry(c, vga_color);
        if (++vga_column == VGA_WIDTH) {
            vga_column = 0;
            if (++vga_row == VGA_HEIGHT && vga_auto_scroll) vga_scroll();
        }
    }
    
    if (current_context) {
        current_context->cursor_x = vga_column;
        current_context->cursor_y = vga_row;
    }
    vga_update_hw_cursor();
}

void vga_print(const char* str) {
    if (!str) return;
    while (*str) vga_putchar(*str++);
}

// ──────────────────────────────
// Advanced graphics operations
// ──────────────────────────────
void vga_draw_box(u16 x, u16 y, u16 width, u16 height, u8 color) {
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) return;
    
    u16 end_x = (x + width > VGA_WIDTH) ? VGA_WIDTH : x + width;
    u16 end_y = (y + height > VGA_HEIGHT) ? VGA_HEIGHT : y + height;
    
    // Draw borders using simple ASCII characters
    for (u16 i = x; i < end_x; i++) {
        if (i == x) {
            vga_print_char(i, y, '+', color);  // Use + for corners
        } else if (i == end_x - 1) {
            vga_print_char(i, y, '+', color);  // Use + for corners
        } else {
            vga_print_char(i, y, '-', color);  // Use - for top border
        }
        
        if (end_y > y + 1) {
            if (i == x) {
                vga_print_char(i, end_y - 1, '+', color);  // Use + for corners
            } else if (i == end_x - 1) {
                vga_print_char(i, end_y - 1, '+', color);  // Use + for corners
            } else {
                vga_print_char(i, end_y - 1, '-', color);  // Use - for bottom border
            }
        }
    }
    
    for (u16 i = y + 1; i < end_y - 1; i++) {
        if (x < end_x) vga_print_char(x, i, '|', color);      // Use | for left border
        if (end_x > x + 1) vga_print_char(end_x - 1, i, '|', color);  // Use | for right border
    }
}


void vga_fill_rect(u16 x, u16 y, u16 width, u16 height, char c, u8 color) {
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) return;
    
    u16 end_x = (x + width > VGA_WIDTH) ? VGA_WIDTH : x + width;
    u16 end_y = (y + height > VGA_HEIGHT) ? VGA_HEIGHT : y + height;
    
    for (u16 cy = y; cy < end_y; cy++) {
        for (u16 cx = x; cx < end_x; cx++) {
            vga_print_char(cx, cy, c, color);
        }
    }
}

void vga_invert_rect(u16 x, u16 y, u16 width, u16 height) {
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) return;
    
    u16 end_x = (x + width > VGA_WIDTH) ? VGA_WIDTH : x + width;
    u16 end_y = (y + height > VGA_HEIGHT) ? VGA_HEIGHT : y + height;
    
    for (u16 cy = y; cy < end_y; cy++) {
        for (u16 cx = x; cx < end_x; cx++) {
            u16 index = vga_get_index(cx, cy);
            struct vga_char ch = vga_buffer[index];
            // Invert foreground and background colors
            u8 fg = ch.color & 0x0F;
            u8 bg = (ch.color >> 4) & 0x0F;
            vga_buffer[index] = vga_entry(ch.character, vga_entry_color(bg, fg));
        }
    }
}

// ──────────────────────────────
// Screen management
// ──────────────────────────────
void vga_save_screen(void) {
    if (current_context) {
        if (!current_context->saved_screen) {
            current_context->saved_screen_size = VGA_WIDTH * VGA_HEIGHT * sizeof(struct vga_char);
            current_context->saved_screen = kmalloc(current_context->saved_screen_size);
        }
        if (current_context->saved_screen) {
            k_memcpy(current_context->saved_screen, vga_buffer, current_context->saved_screen_size);
        }
    }
}

void vga_restore_screen(void) {
    if (current_context && current_context->saved_screen) {
        k_memcpy(vga_buffer, current_context->saved_screen, current_context->saved_screen_size);
    }
}

// ──────────────────────────────
// Utility functions
// ──────────────────────────────
u16 vga_get_cursor_x(void) {
    return current_context ? current_context->cursor_x : vga_column;
}

u16 vga_get_cursor_y(void) {
    return current_context ? current_context->cursor_y : vga_row;
}

u8 vga_get_current_color(void) {
    return current_context ? current_context->color : vga_color;
}

u16 vga_string_width(const char* str) {
    if (!str) return 0;
    u16 len = 0;
    while (*str++) len++;
    return len;
}

void vga_center_text(u16 y, const char* str) {
    if (!str || y >= VGA_HEIGHT) return;
    u16 width = vga_string_width(str);
    u16 x = (VGA_WIDTH - width) / 2;
    vga_print_at(x, y, str);
}

void vga_right_align(u16 y, const char* str) {
    if (!str || y >= VGA_HEIGHT) return;
    u16 width = vga_string_width(str);
    u16 x = VGA_WIDTH - width;
    if (x < VGA_WIDTH) vga_print_at(x, y, str);
}

void vga_left_align(u16 y, const char* str) {
    if (!str || y >= VGA_HEIGHT) return;
    vga_print_at(0, y, str);
}

// ──────────────────────────────
// Context management
// ──────────────────────────────
vga_context_t* vga_create_context(void) {
    vga_context_t* ctx = kmalloc(sizeof(vga_context_t));
    if (ctx) {
        k_memset(ctx, 0, sizeof(vga_context_t));
        ctx->cursor_x = vga_column;
        ctx->cursor_y = vga_row;
        ctx->color = vga_color;
        ctx->is_cursor_visible = vga_cursor_visible;
        ctx->auto_scroll = vga_auto_scroll;
    }
    return ctx;
}

void vga_destroy_context(vga_context_t* ctx) {
    if (ctx && ctx != &default_context) {
        if (ctx->saved_screen) {
            kfree(ctx->saved_screen);
        }
        kfree(ctx);
    }
}

void vga_use_context(vga_context_t* ctx) {
    if (ctx) {
        current_context = ctx;
        vga_column = ctx->cursor_x;
        vga_row = ctx->cursor_y;
        vga_color = ctx->color;
        vga_cursor_visible = ctx->is_cursor_visible;
        vga_auto_scroll = ctx->auto_scroll;
    }
}

void vga_reset_context(void) {
    current_context = &default_context;
    vga_column = default_context.cursor_x;
    vga_row = default_context.cursor_y;
    vga_color = default_context.color;
    vga_cursor_visible = default_context.is_cursor_visible;
    vga_auto_scroll = default_context.auto_scroll;
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
    if (num == 0) {
        vga_putchar('0');
        return;
    }
    char buf[11];
    int i = 10;
    buf[i--] = '\0';
    while (num && i >= 0) {
        buf[i--] = '0' + (num % 10);
        num /= 10;
    }
    vga_print(&buf[i + 1]);
}

static void vga_print_dec_signed(s32 num) {
    if (num < 0) {
        vga_putchar('-');
        num = -num;
    }
    vga_print_dec((u32)num);
}

void vga_vprintf(const char* fmt, va_list args) {
    va_list args_copy;
    va_copy(args_copy, args);
    
    while (*fmt) {
        if (*fmt == '%') {
            ++fmt;
            switch (*fmt) {
                case 's': {
                    const char* str = va_arg(args_copy, const char*);
                    vga_print(str ? str : "(null)");
                    break;
                }
                case 'd': {
                    s32 num = va_arg(args_copy, s32);
                    vga_print_dec_signed(num);
                    break;
                }
                case 'u': {
                    u32 num = va_arg(args_copy, u32);
                    vga_print_dec(num);
                    break;
                }
                case 'x': {
                    u32 num = va_arg(args_copy, u32);
                    vga_print_hex(num);
                    break;
                }
                case 'X': {
                    u32 num = va_arg(args_copy, u32);
                    char hex[9];
                    for (int i = 7; i >= 0; i--) {
                        u8 nibble = num & 0xF;
                        hex[i] = (nibble < 10) ? ('0' + nibble) : ('A' + nibble - 10);
                        num >>= 4;
                    }
                    hex[8] = '\0';
                    vga_print(hex);
                    break;
                }
                case 'p': {
                    u32 num = (u32)va_arg(args_copy, void*);
                    vga_print("0x");
                    vga_print_hex(num);
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args_copy, int);
                    vga_putchar(c);
                    break;
                }
                case 'f': {
                    // Simple float approximation (not very accurate)
                    double f = va_arg(args_copy, double);
                    s32 integer = (s32)f;
                    u32 fractional = (u32)((f - integer) * 100);
                    vga_print_dec_signed(integer);
                    vga_putchar('.');
                    vga_print_dec(fractional);
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
    
    va_end(args_copy);
}

void vga_printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vga_vprintf(fmt, args);
    va_end(args);
}

// ──────────────────────────────
// Hardware Cursor (x86 only)
// ──────────────────────────────
#if defined(__i386__) || defined(__x86_64__)
#define VGA_CMD_PORT  0x3D4
#define VGA_DATA_PORT 0x3D5

void vga_update_hw_cursor(void) {
    if (!vga_cursor_visible) return;
    
    u16 pos = vga_get_index(vga_column, vga_row);
    outb(VGA_CMD_PORT, 0x0F);
    outb(VGA_DATA_PORT, (u8)(pos & 0xFF));
    outb(VGA_CMD_PORT, 0x0E);
    outb(VGA_DATA_PORT, (u8)((pos >> 8) & 0xFF));
}
#else
void vga_update_hw_cursor(void) { /* no-op on RISC-V */ }
#endif