#pragma once

void serial_init(void);
int serial_tx_ready(void);
void serial_putc(char c);
void serial_print(const char* s);
void serial_println(const char* s);
void serial_putbyte(uint8_t b);
void serial_putword(uint16_t w);
void serial_putdword(uint32_t d);