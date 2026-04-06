#include "TFT.h"

void fill_screen(uint16_t *frame_buffer, int color, int pix_size)
{
    for (int i = 0; i < pix_size; i++) {
        *frame_buffer = color;
        frame_buffer ++;
    }
}

void draw_pixel(uint16_t *frame_buffer, int x, int y, int color)
{
    if (!frame_buffer) {
        printf("Frame buffer is nor specify!!!\n");
        return ;
    }
    if (x < 0 || x > SCREEN_HEIGHT) {
        printf("x region need between 0 <= x <= %d\n", SCREEN_HEIGHT);
        return ;
    }
    if (y < 0 || y > SCREEN_WIDTH) {
        printf("y region need between 0 <= y <= %d\n", SCREEN_WIDTH);
        return ;
    }
    printf("draw pixel at x = %d y = %d\n", x, y);
    frame_buffer[x * SCREEN_WIDTH + y] = color;
}

void draw_line(uint16_t *frame_buffer, int x, int y, int width, int height, int color)
{
    if (!frame_buffer) {
        printf("Frame buffer is nor specify!!!\n");
        return ;
    }
    if (x < 0 || x > SCREEN_HEIGHT) {
        printf("x region need between 0 <= x <= %d, x = %d\n", SCREEN_HEIGHT, x);
        return ;
    }
    if (y < 0 || y > SCREEN_WIDTH) {
        printf("y region need between 0 <= y <= %d, y = %d\n", SCREEN_WIDTH, y);
        return ;
    }
    if (width <= 0 || height <= 0) {
        printf("width: %d or height: %d should not <=0", width, height);
        return ;
    }
    printf("draw line!! start at x = %d y = %d\n", x, y);
    for (int w = x; w < x + width; w++) {
        for (int h = y; h < y + height; h++) {
            // printf("w = %d h = %d\n", w, h);
            frame_buffer[w * SCREEN_WIDTH + h] = color;
        }
    }
}

void draw_rect(uint16_t *frame_buffer, int x1, int y1, int x2, int y2, int width, int color, int halo)
{
    if (!frame_buffer) {
        printf("Frame buffer is nor specify!!!\n");
        return ;
    }
    if (x1 < 0 || x2 > SCREEN_HEIGHT || x1 > x2) {
        printf("x region need to 0 <= x1 < x2 <= %d, x1 = %d x2 = %d\n", SCREEN_HEIGHT, x1, x2);
        return ;
    }
    if (y1 < 0 || y2 > SCREEN_WIDTH || y1 > y2) {
        printf("y region need between 0 <= y1 < y2 <= %d, y1 = %d y2 = %d\n", SCREEN_WIDTH, y1, y2);
        return ;
    }
    if (width <= 0) {
        printf("width: %d should not <=0", width);
        return ;
    }
    printf("draw rect!! start at x1 = %d y1 = %d x2 = %d y2 = %d\n", x1, y1, x2, y2);
    for (int w = x1; w < x2; w++) {
        for (int h = y1; h < y2; h++) {
            // printf("w = %d h = %d\n", w, h);
            if (!halo) {
                frame_buffer[w * SCREEN_WIDTH + h] = color;
            }
            else {
                if (w < x1 + width || w > x2 - width || h < y1 + width || h > y2 - width) {
                    frame_buffer[w * SCREEN_WIDTH + h] = color;
                }
            }
        }
    }
}

bmp_info TFT_get_detail_info(uint8_t *buf)
{
    bmp_info bmp;
    bmp.type = (buf[1] << 8) + buf[0];
    bmp.size = (buf[2] + (buf[3] << 8)) + ((buf[4] << 16) + (buf[5] << 24));
    bmp.width = buf[18] + (buf[19] << 8) + (buf[20] << 16) + (buf[21] << 24);
    bmp.height = buf[22] + (buf[23] << 8) + (buf[24] << 16) + (buf[25] << 24); //0x(buf[25])(buf[24])(buf[23])(buf[22])
    bmp.bitcount =  buf[28] + (buf[29] << 8);
    return bmp;
}

int show_bmp(char *file, uint16_t *frame_buffer)
{
    bmp_info bmp;
    char header[54];
    int header_size = 54;
    int bmp_fd = open(file, O_RDWR);
    if (bmp_fd < 0) {
        printf("[Q_Q] bmp file open fail\n");
        return -1;
    }
    if (read(bmp_fd, header, header_size) < 0) {
        printf("read file \"%s\" fail\n", file);
        return -1;
    }
    bmp = TFT_get_detail_info(header);
    printf("%s's msg:\n", file);
    printf("\ttype:%2x\n", bmp.type);
    printf("\tsize:%d\n", bmp.size);
    printf("\tWidth:%d\n", bmp.width);
    printf("\tHeight:%d\n", bmp.height);
    printf("\tBitCount:%d\n", bmp.bitcount);
    if (bmp.type != 0x4d42) {
        printf("The picture is not bmp file\n");
        return -1;
    }

    if (bmp.bitcount != 24) {
        printf("Just support 24bit bmp file\n");
        return -1;
    }


    // skip bmp header
    int offset = header[10] | header[11] << 8 | header[12] << 16 | header[13] << 24;
    lseek(bmp_fd, offset, SEEK_SET);

    // check the size if the bmp need padding.
    int width_size = bmp.width * 3; // 225 * 3(RGB)
    int width_size_padded = width_size;
    if (width_size % 4 != 0) {
        width_size_padded +=  4 - (width_size % 4);
        printf("Need padding, non padded = %d padded = %d !!!\n", width_size, width_size_padded);
    }
    else {
        width_size_padded =  width_size;
        printf("No need to pad!!! %d \n", width_size);
    }
    printf("read %d col\n", bmp.height);
    //Read the bmp file
    char *col_data = malloc(width_size_padded);
    uint16_t *tmp_buf = malloc(bmp.height * bmp.width * sizeof(uint16_t));
    for (int row = 0; row < bmp.height; row++) {
        read(bmp_fd, col_data, width_size_padded);
        for (int col = 0; col < bmp.width; col++) {
            int idx = col * 3;
            uint8_t r = (col_data[idx + 2] >> 3);
            uint8_t g = (col_data[idx + 1] >> 2);
            uint8_t b = col_data[idx];
            uint16_t color = (r << 11) | (g << 5) | (b >> 3);
            // keep the bitmap in a temp buffer.
            if (color == WHITE) {
                tmp_buf[row * bmp.width + col] = BLACK;
            }
            else {
                tmp_buf[row * bmp.width + col] = color;
            }
        }
    }
    //Because of the bitmap in bmp is reverse, so we need to reverse the bitmap.
    for (int row = 0; row < bmp.height; row++) {
        memcpy(&frame_buffer[row * SCREEN_WIDTH],
               &tmp_buf[(bmp.height - 1 - row) * bmp.width],
               bmp.width * sizeof(uint16_t));
    }
    free(tmp_buf);

    return 0;
}

int draw_char_fb(FT_Face face, uint16_t *frame_buffer, int screen_w, int screen_h,
                 int x, int y, char c, uint16_t color)
{
    printf("\n Loading %c\n", c);
    if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
        return -1;
    }

    FT_GlyphSlot g = face->glyph;
    FT_Bitmap *bmp = &g->bitmap;

    for (int row = 0; row < bmp->rows; row++) {
        for (int col = 0; col < bmp->width; col++) {
            int fb_x = x + col + g->bitmap_left;
            int fb_y = y + row - g->bitmap_top + bmp->rows;

            if (fb_x >= 0 && fb_x < screen_w && fb_y >= 0 && fb_y < screen_h) {
                if (bmp->buffer[row * bmp->pitch + col]) {
                    frame_buffer[fb_y * screen_w + fb_x] = color;
                }
            }
        }
    }
    return 0;
}

int draw_text_fb(FT_Face face, uint16_t *frame_buffer, int screen_w, int screen_h,
                 int x, int y, const char *str, uint16_t color)
{
    int pen_x = x;
    int pen_y = y;

    for (const char *p = str; *p; p++) {
        printf("%c ", *p);
        if (draw_char_fb(face, frame_buffer, screen_w, screen_h, pen_x, pen_y, *p, color) == -1) {
            pen_x += 10;
            continue;
        }
        pen_x += face->glyph->advance.x >> 6;
    }
    return 0;
}

FT_Face init_freetype()
{
    FT_Library ft;
    FT_Face face;
    FT_Init_FreeType(&ft);
    FT_New_Face(ft, "/usr/share/fonts/arial.ttf", 0, &face);
    FT_Set_Pixel_Sizes(face, 0, 16);
    return face;
}

/*
int main(int argc, char *argv[])
{
    printf("%d %s\n", argc, argv[0]);
    printf("open Frame Buffer\n");
    static uint16_t *p_frame_buffer = 0;
    int fbfd = open("/dev/fb0", O_RDWR);
    if (!fbfd) {
        printf("[Q_Q]Can't open the framebuffer\n");
        return -1;
    }

    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
        printf("[Q_Q]Error reading fixed information.\n");
        return -1;
    }

    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
        printf("[Q_Q]Error: reading variable information.\n");
        return -1;
    }

    FT_Face face = init_freetype();
    long screensize = finfo.line_length * vinfo.yres;
    printf("screen size %d line length %d yres %d \n", screensize, finfo.line_length, vinfo.yres);
    p_frame_buffer = (uint16_t *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if (p_frame_buffer == -1) {
        printf("[Q_Q]Error: failed to map framebuffer device to memory.\n");
        return -1;
    }

    if (argc > 2 && strcmp(argv[1], "-t") == 0) {
        if (strcmp(argv[2], "r") == 0) {
            printf("Fill red\n");
            fill_screen(p_frame_buffer, RED, RESOLUTION);
        }
        else if (strcmp(argv[2], "g") == 0) {
            printf("Fill green\n");
            fill_screen(p_frame_buffer, GREEN, RESOLUTION);
        }
        else if (strcmp(argv[2], "b") == 0) {
            printf("Fill blue\n");
            fill_screen(p_frame_buffer, BLUE, RESOLUTION);
        }
    }
    else {
        printf("Fill black\n");
        fill_screen(p_frame_buffer, BLACK, RESOLUTION);
    }

    if (argc > 2 && strcmp(argv[1], "-bmp") == 0) {
        printf("Show bmp\n");
        show_bmp(argv[2], p_frame_buffer);
    }

    if (argc > 2 && strcmp(argv[1], "-f") == 0) {
        printf("Show fonts %s\n", argv[2]);
        draw_text_fb(face, p_frame_buffer, SCREEN_WIDTH, SCREEN_HEIGHT, 50, 50, argv[2], 0xFFFF);
    }

    if (argc > 3 && strcmp(argv[1], "-p") == 0) {
        printf("strlen x = %d y = %d\n", strlen(argv[2]), strlen(argv[3]));
        printf("draw pixel x= %d %s y= %d %s\n", str_to_int(argv[2]), argv[2], str_to_int(argv[3]), argv[3]);
        // draw_pixel(p_frame_buffer, str_to_int(argv[2]), str_to_int(argv[3]), RED);
        // draw_pixel(p_frame_buffer, str_to_int(argv[2])+50, str_to_int(argv[3])+50, GREEN);
        // draw_pixel(p_frame_buffer, str_to_int(argv[2])+100, str_to_int(argv[3])+100, BLUE);
        // draw_line(p_frame_buffer, str_to_int(argv[2]), str_to_int(argv[3]), 50, 5, RED);
        draw_rect(p_frame_buffer, str_to_int(argv[2]), str_to_int(argv[3]), str_to_int(argv[4]), str_to_int(argv[5]), 5, RED, 1);
        // draw_rect(p_frame_buffer, 200, 300, 200, 300, 5, BLUE, 0);
    }
    close(fbfd);
}*/