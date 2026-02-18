#include <stdint.h>
#include "io.h"

#define COM1 0x3F8

void serial_init(void) {
    outb(COM1 + 1, 0x00);    // disable interrupts
    outb(COM1 + 3, 0x80);    // DLAB on
    outb(COM1 + 0, 0x03);    // divisor low  (38400 baud if base 115200)
    outb(COM1 + 1, 0x00);    // divisor high
    outb(COM1 + 3, 0x03);    // 8N1
    outb(COM1 + 2, 0xC7);    // enable FIFO, clear, 14-byte threshold
    outb(COM1 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

int serial_tx_ready(void) {
    return inb(COM1 + 5) & 0x20;
}

void serial_putc(char c) {
    while (!serial_tx_ready()) {}
    outb(COM1, (uint8_t)c);
}

void serial_print(const char* s) {
    while (*s) serial_putc(*s++);
}

void serial_println(const char* s) {
    serial_print(s);
    serial_print("\r\n");
}

static char hex_digit(unsigned v) {
    return (v < 10) ? ('0' + v) : ('A' + (v - 10));
}

static inline char high_nibble(unsigned v) {
    return hex_digit((v >> 4) & 0xF);
}
static inline char low_nibble(unsigned v) {
    return hex_digit(v & 0xF);
}

void serial_putbyte(uint8_t b) {
    serial_putc(high_nibble(b));
    serial_putc(low_nibble(b));
}

void serial_putword(uint16_t w) {
    serial_putbyte(w >> 8);
    serial_putbyte(w & 255);
}

void serial_putdword(uint32_t d) {
    serial_putword(d >> 16);
    serial_putword(d & 65535);
}