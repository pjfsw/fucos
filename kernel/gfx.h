#include <stdint.h>

#include "vbemodeinfo.h"
#include "view.h"

#define MEM_FB    0x01000000
#define VIEW_FB   0x01800000
#define FONT_ADDR 0x02000000
void gfxInit(VbeModeInfo *vbeModeInfo);

uint32_t gfxWidth();

uint32_t gfxHeight();

uint32_t gfxBytesPerPixel();

uint32_t *gfxGetFramebuffer();

void gfxFastFill(uint32_t *framebuffer, uint32_t color, uint32_t pixel_count);

void gfxDrawRect(View *v, uint32_t color, uint32_t x, uint32_t y, uint32_t w, uint32_t h);

void gfxRenderView(View *v);

void gfxRender();