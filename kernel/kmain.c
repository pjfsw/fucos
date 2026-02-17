#include "io.h"

void kmain(void) {
    // Text mode poke (B8000). Works without VBE and is immediate gratification.
    volatile unsigned short* vga = (unsigned short*)0xB8000;
    char *startMsg = "FUCOS Futile Crap OS by Johan Fransson";
    int i = 0 ;
    while (startMsg[i] != 0) {
        vga[i] = (unsigned short)(startMsg[i] | (0x0F << 8));
        i++;
    }

    for (;;) { __asm__ __volatile__("hlt"); }
}
