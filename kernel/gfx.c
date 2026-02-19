#include <emmintrin.h> // Include for SSE2 intrinsics
#include "gfx.h"
#include "vbemodeinfo.h"
#include "serial.h"

typedef struct {
    uint32_t *framebuffer;
    uint32_t width;
    uint32_t height;
    uint32_t bytesPerPixel;
    uint16_t *target_fb;
    uint32_t pitch;
} Gfx;

extern void gfxBlit32To16(uint32_t* src, uint16_t* dst, uint32_t width, uint32_t height, uint32_t dst_pitch);

static Gfx gfx;

void gfxInit(VbeModeInfo *vbeModeInfo) {
    serial_print("Graphics: ");
    serial_putword(vbeModeInfo->width);
    serial_putc('x');
    serial_putword(vbeModeInfo->height);
    serial_putc('x');
    serial_putbyte(vbeModeInfo->planes);
    serial_putc('x');
    serial_putbyte(vbeModeInfo->bpp);
    serial_println("");
    serial_print("Framebuffer at ");
    serial_putdword(vbeModeInfo->framebuffer);
    serial_println("");

    gfx.framebuffer = (uint32_t*)0x1000000; // HACK!!!! Video at 16 MB
    gfx.target_fb = (uint16_t*)vbeModeInfo->framebuffer;
    gfx.width = vbeModeInfo->width;
    gfx.height = vbeModeInfo->height;
    gfx.bytesPerPixel = vbeModeInfo->bpp >> 8;
    gfx.pitch = vbeModeInfo->bytes_per_scanline;
}

uint32_t *gfxGetFramebuffer() {
    return gfx.framebuffer;
}

uint32_t gfxWidth() {
    return gfx.width;
}

uint32_t gfxHeight() {
    return gfx.height;
}

uint32_t gfxBytesPerPixel() {
    return gfx.bytesPerPixel;
}

void gfxFastFill(uint32_t color, uint32_t pixel_count) {
    // 1. Create a 128-bit 'vector' containing 4 pixels of our color
    // If color is 0xFF0000 (Red), 'vector' becomes: [Red][Red][Red][Red]
    __m128i vector = _mm_set1_epi32(color);

    // 2. We process 4 pixels per loop iteration (16 bytes)
    // Ensure pixel_count is a multiple of 4!
    for (uint32_t i = 0; i < pixel_count; i += 4) {
        // This is the magic instruction
        // Destination MUST be cast to (__m128i*)
        _mm_stream_si128((__m128i*)&gfx.framebuffer[i], vector);
    }
    
    // 3. Recommended: Tell the CPU to flush the Write-Combining buffers
    // so the pixels actually show up on the screen immediately.
    _mm_sfence(); 
}

static inline uint32_t clamp(uint32_t value, uint32_t low, uint32_t high) {
    if (value < low) {
        value = low;
    } 
    if (value > high) {
        value = high;
    }
    return value;
}

void gfxDrawRect(uint32_t color, uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
    uint32_t x0 = clamp(x, 0, gfx.width - 1);
    uint32_t y0 = clamp(y, 0, gfx.height - 1);
    uint32_t x1 = x + w;
    uint32_t y1 = y + h;
    uint32_t x2 = clamp(x1, 0, gfx.width - 1);
    uint32_t y2 = clamp(y1, 0, gfx.height - 1);

    uint32_t ofs;
    if (y < gfx.height) {
        ofs = x0 + y * gfx.width;    
        for (uint32_t i = x0; i <= x2; i++) {
            gfx.framebuffer[ofs++] = color;
        }
    }
    if (y1 < gfx.height) {
        ofs = x0 + y1 * gfx.width;
        for (uint32_t i = x0; i <= x2; i++) {
            gfx.framebuffer[ofs++] = color;
        }
    }
    if (x < gfx.width) {
        ofs = x + y0 * gfx.width;
        for (uint32_t i = y0; i <= y2; i++) {
            gfx.framebuffer[ofs] = color;
            ofs += gfx.width;
        }
    }    
    if (x1 < gfx.width) {
        ofs = x1 + y0 * gfx.width;
        for (uint32_t i = y0; i <= y2; i++) {
            gfx.framebuffer[ofs] = color;
            ofs += gfx.width;
        }
    }
}

void gfxRender() {
    serial_println("Render");
    gfxBlit32To16(gfx.framebuffer, gfx.target_fb, gfx.width, gfx.height, gfx.pitch);
}