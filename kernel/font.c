#include "font.h"
#include "allocator.h"

/**
 * Unpacks a 1-bit font into an 8-bit per pixel array.
 * @param font_source  The 1-bit packed font data (256 * 16 bytes)
 * @param target_array The destination buffer (256 * 8 * 16 bytes)
 */
static void unpack_font_1bit_to_8bit(const uint8_t *font_source, uint32_t *target_array) {
    for (int c = 0; c < FONT_CHARS; c++) {
        for (int y = 0; y < FONT_HEIGHT; y++) {
            // Each row in an 8-wide font is exactly 1 byte
            uint8_t row_data = font_source[c * FONT_HEIGHT + y];

            for (int x = 0; x < FONT_WIDTH; x++) {
                // Calculate the index in the flat target array
                int target_idx = (c * FONT_WIDTH * FONT_HEIGHT) + (y * FONT_WIDTH) + x;

                // Extract the bit. 
                // Note: (7 - x) assumes the MSB (Most Significant Bit) is the leftmost pixel.
                if (row_data & (1 << (7 - x))) {
                    target_array[target_idx] = 1;
                } else {
                    target_array[target_idx] = 0;
                }
            }
        }
    }
}

static const unsigned char packed_font_data[] = {
    #embed "IBM_VGA_8x16.bin"
};

void fontInit(Font *font) {
    font->data = allocate(FONT_WIDTH * FONT_HEIGHT * FONT_CHARS * 4);
    unpack_font_1bit_to_8bit(packed_font_data, font->data);
}

static inline void printChar(Font *font, View *view, char c, int x, int y, uint32_t color) {
    // 1. Correct the offset calculation
    int srcofs = c * (FONT_WIDTH * FONT_HEIGHT);

    for (int yy = 0; yy < FONT_HEIGHT; yy++) {
        for (int xx = 0; xx < FONT_WIDTH; xx++) {
            uint32_t on = font->data[srcofs + (yy * FONT_WIDTH) + xx];

            if (on) {
                // 2. Add xx and yy to the destination coordinates
                int destX = x + xx;
                int destY = y + yy;

                // 3. Safety check: Don't write outside the framebuffer
                if (destX < view->w && destY < view->h) {
                    view->framebuffer[destY * view->w + destX] = color;
                }
            }
        }
    }
}
void print(Font *font, View *view, char *txt, int x, int y, uint32_t color) {
    while (txt[0] != 0) {
        printChar(font, view, txt[0], x, y, color);
        x += FONT_WIDTH;
        txt++;
    }
}