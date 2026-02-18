#include <stdint.h>

#include "idt.h"
#include "serial.h"
#include "textmode.h"
#include "io.h"
#include "vbemodeinfo.h"

#define PIC1_CMD   0x20
#define PIC1_DATA  0x21
#define PIC2_CMD   0xA0
#define PIC2_DATA  0xA1
#define PIC_EOI    0x20

static void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) outb(PIC2_CMD, PIC_EOI);
    outb(PIC1_CMD, PIC_EOI);
}

static void pic_remap(uint8_t off1, uint8_t off2) {
    uint8_t a1 = inb(PIC1_DATA);
    uint8_t a2 = inb(PIC2_DATA);

    outb(PIC1_CMD, 0x11);
    outb(PIC2_CMD, 0x11);

    outb(PIC1_DATA, off1);
    outb(PIC2_DATA, off2);

    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);

    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a2);
}

static void pic_unmask_irq(uint8_t irq) {
    uint16_t port = (irq < 8) ? PIC1_DATA : PIC2_DATA;
    if (irq >= 8) irq -= 8;
    uint8_t mask = inb(port);
    mask &= (uint8_t)~(1u << irq);
    outb(port, mask);
}

static void pit_init(uint32_t hz) {
    uint32_t div = 1193182u / hz;
    outb(0x43, 0x36);              // channel 0, lobyte/hibyte, mode 3
    outb(0x40, (uint8_t)(div & 0xFF));
    outb(0x40, (uint8_t)((div >> 8) & 0xFF));
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

static volatile uint32_t ticks = 0;

void isr_handler(registers_t *regs) {
    uint32_t int_no = regs->int_no;

    if (int_no < 32) {
        __asm__ __volatile__("cli");
        serial_print("EXC ");
        serial_putc(hex_digit((int_no >> 4) & 0xF));
        serial_putc(hex_digit(int_no & 0xF));
        serial_print("\r\n");
        for (;;) { __asm__ __volatile__("hlt"); }
    }

    uint8_t irq = (uint8_t)(int_no - 32);

    if (irq == 0) {
        ticks++;
        if ((ticks % 100) == 0) {
            int s = ticks / 100;
            vga_putc(high_nibble(s % 60), 2, 7);
            vga_putc(low_nibble(s % 60), 2, 8);
        }
        //serial_println("tick");

    } else if (irq == 1) {
        uint8_t sc = inb(0x60);
        serial_print("kbd ");
        serial_putc(hex_digit((sc >> 4) & 0xF));
        serial_putc(hex_digit(sc & 0xF));
        serial_print("\r\n");
    }

    pic_send_eoi(irq);
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

void put_hex8(uint8_t b) {
    serial_putc(high_nibble(b));
    serial_putc(low_nibble(b));
}

void put_hex16(uint16_t w) {
    put_hex8((uint8_t)(w >> 8));
    put_hex8((uint8_t)(w & 255));
}

void put_hex32(uint32_t d) {
    put_hex16((uint16_t)(d >> 16));
    put_hex16((uint16_t)(d & 65535));
}

void draw_a_rainbow(VbeModeInfo *vbeModeInfo) {
    uint16_t *fb = (uint16_t*)vbeModeInfo->framebuffer;
    for (int i = 0; i < 800*600; i++) {
        fb[i] = i;
    }
}

void kmain(VbeModeInfo *vbeModeInfo) {
    serial_init();
    serial_println("FUCOS BOOT START");

    idt_init();
    serial_println("Idt active");

    // prove we’re alive
    vga_disable_cursor();
    vga_clear();
    vga_puts("FUCOS", 1, 0);
    vga_puts("Timer: ", 2, 0);

    serial_println("Testing A20");
    if (a20_enabled()) {
        serial_println("A20 enabled");
    } else {
        serial_println("A20 NOT enabled");
    }

    put_hex16(vbeModeInfo->width);
    serial_putc('x');
    put_hex16(vbeModeInfo->height);
    serial_putc('x');
    put_hex8(vbeModeInfo->planes);
    serial_putc('x');
    put_hex8(vbeModeInfo->bpp);
    serial_println("");

    put_hex32(vbeModeInfo->framebuffer);

    draw_a_rainbow(vbeModeInfo);

    pic_remap(0x20, 0x28);
    pit_init(100);          // 100 Hz
    pic_unmask_irq(0);      // timer
    pic_unmask_irq(1);      // keyboard

     __asm__ __volatile__("sti");

    for (;;) { __asm__ __volatile__("hlt"); }
}