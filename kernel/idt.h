#pragma once
#include <stdint.h>

typedef struct {
   uint32_t ds;                                     // Data segment selector
   uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pushad
   uint32_t int_no, err_code;                       // Pushed by our macros
   uint32_t eip, cs, eflags, useresp, ss;           // Pushed by the processor automatically
} registers_t;

void idt_init(void);
void isr_handler(registers_t *regs);
