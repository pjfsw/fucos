#include <emmintrin.h> // Include for SSE2 intrinsics
#include "gfx.h"
#include "vbemodeinfo.h"
#include "serial.h"
#include "mtrr.h"

typedef struct {
    uint32_t *framebuffer;
    uint32_t width;
    uint32_t height;
    uint32_t bpp;
    uint16_t *target_fb_16;
    uint32_t *target_fb_32;
    uint32_t pitch;
} Gfx;

extern void gfxBlit32To16(uint32_t* src, uint16_t* dst, uint32_t width, uint32_t height, uint32_t dst_pitch);
extern void gfxBlit32To32(uint32_t* src, uint32_t* dst, uint32_t width, uint32_t height, uint32_t dst_pitch);

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
    gfx.target_fb_16 = (uint16_t*)vbeModeInfo->framebuffer;
    gfx.target_fb_32 = (uint32_t*)vbeModeInfo->framebuffer;
    gfx.width = vbeModeInfo->width;
    gfx.height = vbeModeInfo->height;
    gfx.bpp = vbeModeInfo->bpp;
    gfx.pitch = vbeModeInfo->bytes_per_scanline;

    // Next power of two after 960,000 bytes is 1MB (0x100000)
    uint32_t wc_size = 0x100000;

    // Enable Write-Combining for the hardware framebuffer
    mtrr_set_wc(vbeModeInfo->framebuffer, wc_size);
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

uint32_t gfxBitsPerPixel() {
    return gfx.bpp;
}

uint32_t *getFrameBuffer() {
    return gfx.framebuffer;    
}

void gfxFastFill(uint32_t *framebuffer, uint32_t color, uint32_t pixel_count) {
    // 1. Create a 128-bit 'vector' containing 4 pixels of our color
    // If color is 0xFF0000 (Red), 'vector' becomes: [Red][Red][Red][Red]
    __m128i vector = _mm_set1_epi32(color);

    // 2. We process 4 pixels per loop iteration (16 bytes)
    // Ensure pixel_count is a multiple of 4!
    for (uint32_t i = 0; i < pixel_count; i += 4) {
        // This is the magic instruction
        // Destination MUST be cast to (__m128i*)
        _mm_stream_si128((__m128i*)&framebuffer[i], vector);
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

void gfxDrawRect(View *v, uint32_t color, uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
    uint32_t x0 = clamp(x, 0, v->w - 1);
    uint32_t y0 = clamp(y, 0, v->virtual_h - 1);
    uint32_t x1 = x + w;
    uint32_t y1 = y + h;
    uint32_t x2 = clamp(x1, 0, v->w - 1);
    uint32_t y2 = clamp(y1, 0, v->virtual_h - 1);

    uint32_t ofs;
    if (y < v->virtual_h) {
        ofs = x0 + y * v->w;    
        for (uint32_t i = x0; i <= x2; i++) {
            v->framebuffer[ofs++] = color;
        }
    }
    if (y1 < v->virtual_h) {
        ofs = x0 + y1 * v->w;
        for (uint32_t i = x0; i <= x2; i++) {
            v->framebuffer[ofs++] = color;
        }
    }
    if (x < v->w) {
        ofs = x + y0 * v->w;
        for (uint32_t i = y0; i <= y2; i++) {
            v->framebuffer[ofs] = color;
            ofs += v->w;
        }
    }    
    if (x1 < v->w) {
        ofs = x1 + y0 * v->w;
        for (uint32_t i = y0; i <= y2; i++) {
            v->framebuffer[ofs] = color;
            ofs += v->w;
        }
    }
}

// Copies a row of 32-bit pixels using 128-bit SSE chunks.
// Constraint: width_pixels MUST be a multiple of 4.
static inline void sse_copy_row(uint32_t *dst, uint32_t *src, uint32_t width_pixels) {
    uint32_t chunks = width_pixels / 4; // 4 pixels per SSE register
    
    __m128i *d = (__m128i *)dst;
    __m128i *s = (__m128i *)src;
    
    for (uint32_t i = 0; i < chunks; i++) {
        // _mm_loadu_si128  = movdqu (Unaligned load)
        // _mm_storeu_si128 = movdqu (Unaligned store)
        _mm_storeu_si128(&d[i], _mm_loadu_si128(&s[i]));
    }
}

void gfxRenderView(View *v) {
    if (!v->dirty) {
        return;
    }

    // We assume scroll_y is always strictly less than virtual_h.
    // If scroll_y reaches virtual_h, you simply reset it to 0.

    uint32_t part1_h = v->virtual_h - v->scroll_y;
    uint32_t part2_h = 0;

    // Check if the visible area wraps around the end of the buffer
    if (part1_h >= v->h) {
        // No wrap needed! The visible area fits entirely before the buffer ends.
        part1_h = v->h; 
    } else {
        // Wrap needed! Part 1 hits the bottom, Part 2 starts at the top.
        part2_h = v->h - part1_h;
    }

// --- BLIT PART 1 ---
    uint32_t *src1 = v->framebuffer + (v->scroll_y * v->w);
    uint32_t *dst1 = gfx.framebuffer + (v->y * gfx.width) + v->x;
    
    for (uint32_t row = 0; row < part1_h; row++) {
        sse_copy_row(dst1 + (row * gfx.width), src1 + (row * v->w), v->w);
    }

    // --- BLIT PART 2 (The Wrap) ---
    if (part2_h > 0) {
        uint32_t *src2 = v->framebuffer;
        uint32_t *dst2 = gfx.framebuffer + ((v->y + part1_h) * gfx.width) + v->x;

        for (uint32_t row = 0; row < part2_h; row++) {
            sse_copy_row(dst2 + (row * gfx.width), src2 + (row * v->w), v->w);
        }
    }    

    v->dirty = false;
}


void gfxRender() {
    if (gfx.bpp == 32) {
        gfxBlit32To32(gfx.framebuffer, gfx.target_fb_32, gfx.width, gfx.height, gfx.pitch);
    } else if (gfx.bpp == 16) {
        gfxBlit32To16(gfx.framebuffer, gfx.target_fb_16, gfx.width, gfx.height, gfx.pitch);
    } else {
        serial_print("Render: Unsupported bpp: ");
        serial_putdword(gfx.bpp);
        serial_println("");
    }
}