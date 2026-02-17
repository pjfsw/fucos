#include "idt.h"
#include "serial.h"

static void vga_puts(const char* s, int col) {
    volatile unsigned short* vga = (unsigned short*)0xB8000;
    while (*s) {
        vga[col++] = (unsigned short)(*s++ | (0x0F << 8));
    }
}

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
    serial_println("FUCOS BOOT");

    idt_init();

    // prove we’re alive
    vga_puts("FUCOS", 0);

    trigger_de();

    for (;;) { __asm__ __volatile__("hlt"); }
}