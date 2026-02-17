#pragma once

#include <stdint.h>

void vga_puts(const char* s, int row, int col);

void vga_clear();

void vga_disable_cursor(void);

void vga_set_cursor(uint8_t row, uint8_t col);