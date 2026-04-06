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

#define RED 0xf800
#define GREEN 0x07e0
#define BLUE 0x001f
#define BLACK  0x0000
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 480
#define RESOLUTION SCREEN_WIDTH*SCREEN_HEIGHT

struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;

typedef struct s_bmp_info{
    uint16_t type;
    uint32_t size;
	uint32_t width;
	uint32_t height;
	uint16_t bitcount;	
}bmp_info;

void fill_screen(uint16_t *fb_men, int color, int pix_size);
int show_bmp(char *file, uint16_t *frame_buffer);

int draw_text_fb(FT_Face face, uint16_t *fb, int screen_w, int screen_h,
                 int x, int y, const char *str, uint16_t color);