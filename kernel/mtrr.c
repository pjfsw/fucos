#include "mtrr.h"

#define MTRR_DEF_TYPE_MSR  0x2FF
#define MTRR_PHYS_BASE_0   0x200
#define MTRR_PHYS_MASK_0   0x201

static inline void rdmsr(uint32_t msr, uint32_t *lo, uint32_t *hi) {
    __asm__ volatile("rdmsr" : "=a"(*lo), "=d"(*hi) : "c"(msr));
}

static inline void wrmsr(uint32_t msr, uint32_t lo, uint32_t hi) {
    __asm__ volatile("wrmsr" : : "a"(lo), "d"(hi), "c"(msr));
}

void mtrr_set_wc(uint32_t base_addr, uint32_t size) {
    uint32_t lo, hi;
    
    // 1. Check if MTRRs are enabled on the CPU
    rdmsr(MTRR_DEF_TYPE_MSR, &lo, &hi);
    if (!(lo & (1 << 11))) {
        // MTRRs are disabled globally, we can't do this.
        return; 
    }

    // 2. Find a free Variable MTRR (CPUs usually have 8 pairs, from 0x200 to 0x20F)
    int free_mtrr = -1;
    for (int i = 0; i < 8; i++) {
        rdmsr(MTRR_PHYS_MASK_0 + (i * 2), &lo, &hi);
        if ((lo & (1 << 11)) == 0) { // Check the 'Valid' bit (Bit 11)
            free_mtrr = i;
            break;
        }
    }

    if (free_mtrr == -1) {
        // No free MTRRs found! 
        return;
    }

    // 3. Calculate the Mask (This requires the size to be a power of 2!)
    // Formula: ~(size - 1) & PhysicalAddressMask
    // We only care about 32-bit physical addresses right now.
    uint32_t mask = ~(size - 1);

    // 4. Write the Base address and Type (Type 1 = Write Combining)
    // base_addr must be aligned to 'size'. VESA framebuffers usually are.
    uint32_t base_msr = MTRR_PHYS_BASE_0 + (free_mtrr * 2);
    wrmsr(base_msr, base_addr | 1, 0); // 1 = WC type

    // 5. Write the Mask and enable the 'Valid' bit (Bit 11)
    uint32_t mask_msr = MTRR_PHYS_MASK_0 + (free_mtrr * 2);
    wrmsr(mask_msr, mask | (1 << 11), 0);
}