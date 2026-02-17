#pragma once
#include <stdint.h>

void idt_init(void);
void isr_handler(uint32_t int_no, uint32_t err_code);
