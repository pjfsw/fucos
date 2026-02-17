#include <stdint.h>

#include "idt.h"
#include "serial.h"
#include "textmode.h"

static char hex_digit(unsigned v) {
    return (v < 10) ? ('0' + v) : ('A' + (v - 10));
}

void isr_handler(uint32_t int_no, uint32_t err_code) {
    __asm__ __volatile__("cli");
    static volatile int in_exception = 0;

    if (in_exception++) {
        for (;;)
            __asm__ __volatile__("hlt");
    }

    serial_print("EXC ");
    serial_putc(hex_digit((int_no >> 4) & 0xF));
    serial_putc(hex_digit(int_no & 0xF));
    serial_print("\r\n");

    // Print "EXC XX" at top-left
    volatile unsigned short* vga = (unsigned short*)0xB8000;
    const char* p = "EXC ";
    int i = 0;
    while (p[i]) { vga[i] = (unsigned short)(p[i] | (0x4F << 8)); i++; }

    char a = hex_digit((int_no >> 4) & 0xF);
    char b = hex_digit(int_no & 0xF);
    vga[i++] = (unsigned short)(a | (0x4F << 8));
    vga[i++] = (unsigned short)(b | (0x4F << 8));
    
    for (;;) { __asm__ __volatile__("cli; hlt"); }
}

int a20_enabled(void)
{
    volatile uint8_t *low  = (uint8_t*)0x000500;
    volatile uint8_t *high = (uint8_t*)0x100500; // 1 MiB

    uint8_t old_low  = *low;
    uint8_t old_high = *high;

    *low  = 0xAA;
    *high = 0x55;

    int enabled = (*low != *high);

    // restore memory
    *low  = old_low;
    *high = old_high;

    return enabled;
}


static inline void trigger_de(void) {
    __asm__ volatile(
        "xor %%edx, %%edx \n"
        "mov $1, %%eax    \n"
        "xor %%ecx, %%ecx \n"  // ecx = 0
        "div %%ecx        \n"  // => #DE
        :
        :
        : "eax", "ecx", "edx"
    );
}

void kmain(void) {
    serial_init();
    serial_println("FUCOS BOOT START");

    idt_init();
    serial_println("Idt active");

    // prove we’re alive
    vga_disable_cursor();
    vga_clear();
    vga_puts("FUCOS", 1, 0);
    vga_puts("Another line", 2, 0);

    serial_println("Testing A20");
    if (a20_enabled()) {
        serial_println("A20 enabled");
    } else {
        serial_println("A20 NOT enabled");
    }

    //trigger_de();

    for (;;) { __asm__ __volatile__("hlt"); }
}