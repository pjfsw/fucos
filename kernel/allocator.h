#pragma once

#include <stdint.h>
#include "sysinfo.h"

void allocator_init(MemoryMap *memoryMap);

void *allocate(uint32_t bytes);