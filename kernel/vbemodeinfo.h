#pragma once

#include <stdint.h>

typedef struct {
    uint16_t attributes;
    uint8_t  winA_attributes;
    uint8_t  winB_attributes;
    uint16_t win_granularity;
    uint16_t win_size;
    uint16_t winA_segment;
    uint16_t winB_segment;
    uint32_t win_func_ptr;
    uint16_t bytes_per_scanline;

    // VBE 1.2+
    uint16_t width;
    uint16_t height;
    uint8_t  x_char_size;
    uint8_t  y_char_size;
    uint8_t  planes;
    uint8_t  bpp;               // Bits per pixel
    uint8_t  banks;
    uint8_t  memory_model;
    uint8_t  bank_size;
    uint8_t  image_pages;
    uint8_t  reserved0;

    // Direct Color fields
    uint8_t  red_mask_size;
    uint8_t  red_field_position;
    uint8_t  green_mask_size;
    uint8_t  green_field_position;
    uint8_t  blue_mask_size;
    uint8_t  blue_field_position;
    uint8_t  reserved_mask_size;
    uint8_t  reserved_field_position;
    uint8_t  direct_color_mode_info;

    // VBE 2.0+
    uint32_t framebuffer;       // Physical address of the LFB
    uint32_t off_screen_mem_off;
    uint16_t off_screen_mem_size;
    
    uint8_t  reserved1[206];
} __attribute__((packed)) VbeModeInfo;