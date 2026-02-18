#include <stdint.h>

#include "vbemodeinfo.h"

#define GFX_WIDTH 1280
#define GFX_HEIGHT 1024

void gfxInit(VbeModeInfo *vbeModeInfo);

uint32_t gfxWidth();

uint32_t gfxHeight();

uint32_t gfxBytesPerPixel();

void gfxFastFill(uint32_t color, uint32_t pixel_count);

void gfxDrawRect(uint32_t color, uint32_t x, uint32_t y, uint32_t w, uint32_t h);