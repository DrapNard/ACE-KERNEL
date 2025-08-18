#include "../include/vga.h"
#include "../include/memory.h"

static struct vga_char* vga_buffer = (struct vga_char*)VGA_MEMORY;
static u16 vga_row = 0;
static u16 vga_column = 0;
static u8 vga_color = 0;

static u8 vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

static struct vga_char vga_entry(unsigned char uc, u8 color) {
    struct vga_char entry;
    entry.character = uc;
    entry.color = color;
    return entry;
}

void vga_init(void) {
    vga_row = 0;
    vga_column = 0;
    vga_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_clear();
}

void vga_clear(void) {
    for (u16 y = 0; y < VGA_HEIGHT; y++) {
        for (u16 x = 0; x < VGA_WIDTH; x++) {
            const u16 index = y * VGA_WIDTH + x;
            vga_buffer[index] = vga_entry(' ', vga_color);
        }
    }
    vga_row = 0;
    vga_column = 0;
}

void vga_set_color(enum vga_color fg, enum vga_color bg) {
    vga_color = vga_entry_color(fg, bg);
}

void vga_set_cursor(u16 x, u16 y) {
    if (x < VGA_WIDTH && y < VGA_HEIGHT) {
        vga_column = x;
        vga_row = y;
    }
}

void vga_scroll(void) {
    for (u16 y = 0; y < VGA_HEIGHT - 1; y++) {
        for (u16 x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = vga_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }

    for (u16 x = 0; x < VGA_WIDTH; x++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', vga_color);
    }

    vga_row = VGA_HEIGHT - 1;
}

void vga_putchar(char c) {
    if (c == '\n') {
        vga_column = 0;
        if (++vga_row == VGA_HEIGHT) {
            vga_scroll();
        }
        return;
    }

    if (c == '\r') {
        vga_column = 0;
        return;
    }

    if (c == '\t') {
        vga_column = (vga_column + 8) & ~(8 - 1);
        if (vga_column >= VGA_WIDTH) {
            vga_column = 0;
            if (++vga_row == VGA_HEIGHT) {
                vga_scroll();
            }
        }
        return;
    }

    const u16 index = vga_row * VGA_WIDTH + vga_column;
    vga_buffer[index] = vga_entry(c, vga_color);

    if (++vga_column == VGA_WIDTH) {
        vga_column = 0;
        if (++vga_row == VGA_HEIGHT) {
            vga_scroll();
        }
    }
}

void vga_print(const char* str) {
    if (!str) return;

    while (*str) {
        vga_putchar(*str++);
    }
}
