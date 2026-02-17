#include "io.h"

void vga_puts(const char* s, int row, int col) {
    volatile unsigned short* vga = (unsigned short*)0xB8000;
    int ofs = row * 80 + col;
    while (*s) {
        vga[ofs++] = (unsigned short)(*s++ | (0x0F << 8));
    }
}

void vga_clear() {
    volatile unsigned short* vga = (unsigned short*)0xB8000;
    for (int i = 0; i < 80*30; i++) {
        vga[i] = (unsigned short)(' ' | (0x0F << 8));
    }
}

void vga_disable_cursor(void) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

// Move cursor to (row, col)
void vga_set_cursor(uint8_t row, uint8_t col) {
    uint16_t pos = (uint16_t)row * 80 + col;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}