#pragma once

#include "vbemodeinfo.h"

#define MAX_E820_ENTRIES 32

typedef struct {
    uint64_t base;      // physical start address
    uint64_t length;    // size of region in bytes
    uint32_t type;      // 1 = usable RAM
    uint32_t attr;      // extended attributes (ACPI 3.0+)
} __attribute__((packed)) E820Entry;

typedef struct {
    uint32_t e820_count;
    E820Entry e820[MAX_E820_ENTRIES];    
}  __attribute__((packed))  MemoryMap;

typedef struct {
    VbeModeInfo vbeModeInfo;
    MemoryMap memoryMap;    
} __attribute__((packed))  SysInfo;