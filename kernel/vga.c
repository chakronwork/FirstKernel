#include "vga.h"

#define VGA_W 80
#define VGA_H 25

static uint16_t *const buf = (uint16_t *)0xB8000;
static size_t row = 0;
static size_t col = 0;
static uint8_t color = 0;

static inline void vga_outb(uint16_t port, uint8_t val)
{
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t vga_inb(uint16_t port)
{
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline uint16_t entry(char c, uint8_t co)
{
    return (uint16_t)(unsigned char)c | ((uint16_t)co << 8);
}

static void vga_update_cursor(size_t x, size_t y)
{
    uint16_t pos = (uint16_t)(y * VGA_W + x);

    vga_outb(0x3D4, 0x0F);
    vga_outb(0x3D5, (uint8_t)(pos & 0xFF));
    vga_outb(0x3D4, 0x0E);
    vga_outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

static void vga_enable_cursor(uint8_t cursor_start, uint8_t cursor_end)
{
    vga_outb(0x3D4, 0x0A);
    vga_outb(0x3D5, (vga_inb(0x3D5) & 0xC0) | cursor_start);

    vga_outb(0x3D4, 0x0B);
    vga_outb(0x3D5, (vga_inb(0x3D5) & 0xE0) | cursor_end);
}

void vga_setcolor(enum vga_color foreground, enum vga_color background)
{
    color = (uint8_t)foreground | ((uint8_t)background << 4);
}

static void scroll(void)
{
    for (size_t y = 1; y < VGA_H; y++) {
        for (size_t x = 0; x < VGA_W; x++) {
            buf[(y - 1) * VGA_W + x] = buf[y * VGA_W + x];
        }
    }

    for (size_t x = 0; x < VGA_W; x++) {
        buf[(VGA_H - 1) * VGA_W + x] = entry(' ', color);
    }

    row = VGA_H - 1;
}

void vga_init(void)
{
    row = 0;
    col = 0;

    vga_setcolor(VGA_LGREY, VGA_BLACK);

    for (size_t y = 0; y < VGA_H; y++) {
        for (size_t x = 0; x < VGA_W; x++) {
            buf[y * VGA_W + x] = entry(' ', color);
        }
    }

    vga_enable_cursor(14, 15);
    vga_update_cursor(col, row);
}

void vga_putc(char c)
{
    if (c == '\r') {
        col = 0;
        vga_update_cursor(col, row);
        return;
    }

    if (c == '\n') {
        col = 0;
        row++;

        if (row >= VGA_H) {
            scroll();
        }

        vga_update_cursor(col, row);
        return;
    }

    if (c == '\t') {
        col = (col + 4) & ~3U;

        if (col >= VGA_W) {
            col = 0;
            row++;

            if (row >= VGA_H) {
                scroll();
            }
        }

        vga_update_cursor(col, row);
        return;
    }

    buf[row * VGA_W + col] = entry(c, color);
    col++;

    if (col >= VGA_W) {
        col = 0;
        row++;

        if (row >= VGA_H) {
            scroll();
        }
    }

    vga_update_cursor(col, row);
}

void vga_write(const char *s)
{
    if (s == 0)
        return;

    while (*s) {
        vga_putc(*s++);
    }
}

void vga_backspace(void)
{
    if (col == 0) {
        return;
    }

    col--;
    buf[row * VGA_W + col] = entry(' ', color);
    vga_update_cursor(col, row);
}
