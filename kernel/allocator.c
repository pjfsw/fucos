#include "allocator.h"
#include "serial.h"

extern uint8_t _kernel_start[];
extern uint8_t _kernel_end[];

static uint32_t next_ptr;

#define PAGE_SIZE 4096
#define PAGE_ALIGN_UP(addr)   (((addr) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define PAGE_ALIGN_DOWN(addr) ((addr) & ~(PAGE_SIZE - 1))

void allocator_init(MemoryMap *memoryMap) {
    serial_println("Memory map:");
    for (int i = 0 ; i < memoryMap->e820_count; i++) {
        E820Entry *entry = &memoryMap->e820[i];
        if (entry->type == 1) {
            serial_putdword(entry->base);
            serial_putc(' ');
            serial_putdword(entry->length);
            serial_println("");
        }
    }
    uint32_t kernel_size = _kernel_end - _kernel_start;
    serial_print("Kernel size: ");
    serial_putdword(kernel_size);
    serial_println("");

    next_ptr = 0x100000 + kernel_size;        
}

void *allocate(uint32_t bytes) {
    void *ptr = (void*)next_ptr;
    uint32_t to_allocate = PAGE_ALIGN_UP(bytes);
    serial_print("Allocate ");
    serial_putdword(to_allocate);
    serial_print(" bytes at ");
    serial_putdword(next_ptr);
    serial_println("");
    next_ptr += to_allocate;
    return ptr;
}