#pragma once

#include <stdbool.h>

typedef struct {
    // --- Screen Placement ---
    uint32_t x;             // X position on the main screen
    uint32_t y;             // Y position on the main screen
    uint32_t w;             // Visible width in pixels
    uint32_t h;             // Visible height in pixels
    
    // --- Local Storage ---
    uint32_t *framebuffer;  // The View's private 32-bit pixel buffer
    
    // --- Scrolling Mechanics ---
    uint32_t virtual_h;     // How tall the backing buffer ACTUALLY is
    uint32_t scroll_y;      // Which row of the virtual buffer is currently at the top

    bool dirty;             // OPTIMIZATION: Did this view change since last frame?

} View;    
