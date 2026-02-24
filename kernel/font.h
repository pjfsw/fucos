#pragma once

#include <stdint.h>
#include "view.h"

#define FONT_WIDTH 8
#define FONT_HEIGHT 16
#define FONT_CHARS 256

typedef struct {
    uint32_t *data;
} Font;

void fontInit(Font *font);

void print(Font *font, View *view, char *txt, int x, int y, uint32_t color);