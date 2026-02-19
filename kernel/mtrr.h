#pragma once

#include <stdint.h>

/* * MTRRs (Memory Type Range Registers) tell the CPU how to handle physical memory.
 * By default, VRAM is "Uncacheable", so the CPU writes to it one agonizing byte at a time.
 * Setting it to "Write-Combining" (WC) tells the CPU to gather pixels into 64-byte chunks 
 * and blast them across the bus in bursts, which is required for 60fps on real hardware.
 */
void mtrr_set_wc(uint32_t base_addr, uint32_t size);