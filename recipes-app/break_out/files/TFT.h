#pragma once
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <errno.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "utils.h"

#define RED 0xf800
#define GREEN 0x07e0
#define BLUE 0x001f
#define BLACK  0x0000
#define WHITE  0xffff
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 480
#define RESOLUTION SCREEN_WIDTH*SCREEN_HEIGHT

typedef struct s_bmp_info {
    uint16_t type;
    uint32_t size;
    uint32_t width;
    uint32_t height;
    uint16_t bitcount;
} bmp_info;

void fill_screen(uint16_t *frame_buffer, int color, int pix_size);
bmp_info TFT_get_detail_info(uint8_t *buf);
int show_bmp(char *file, uint16_t *frame_buffer);
int draw_char_fb(FT_Face face, uint16_t *frame_buffer, int screen_w, int screen_h,
                 int x, int y, char c, uint16_t color);
int draw_text_fb(FT_Face face, uint16_t *frame_buffer, int screen_w, int screen_h,
                 int x, int y, const char *str, uint16_t color);
FT_Face init_freetype();
void draw_pixel(uint16_t *frame_buffer, int x, int y, int color);
void draw_line(uint16_t *frame_buffer, int x, int y, int width, int height, int color);
void draw_rect(uint16_t *frame_buffer, int x1, int x2, int y1, int y2, int width, int color, int halo);