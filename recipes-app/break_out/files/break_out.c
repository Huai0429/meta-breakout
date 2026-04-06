#include "TFT.h"
#include <stdlib.h>

#define GAME_REGION 400
#define BRICK_HEIGHT 5
#define BRICK_WDITH 7

#define STICK_HEIGHT 12
#define STICK_WIDTH 2

#define GAP_WIDTH 1
#define REGION_WIDTH 5

#define BALL_SIZE 3
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;

int generate_brick(uint16_t *p_frame_buffer, int brick_num)
{

    int cnt = 0;
    for (int row_pos = GAP_WIDTH ; row_pos < GAME_REGION; row_pos += (BRICK_HEIGHT + GAP_WIDTH)) {
        for (int col_pos = GAP_WIDTH; col_pos < SCREEN_WIDTH - (BRICK_WDITH + GAP_WIDTH) ; col_pos += (BRICK_WDITH + GAP_WIDTH)) {
            if (rand() % 2 == 0) {
                cnt++;
                printf("brick at (%d,%d)\n", row_pos, col_pos);
                draw_line(p_frame_buffer, row_pos, col_pos, BRICK_HEIGHT, BRICK_WDITH, WHITE); //brick
            }
            if (cnt == brick_num) {
                printf("brick over limit %d \n", brick_num);
                return 0;
            }
        }
    }
    printf("brick num = %d\n", cnt);
    return 0;
}
int init_game_board(uint16_t *p_frame_buffer)
{
    printf("clear the screen\n");
    fill_screen(p_frame_buffer, BLACK, RESOLUTION);
    // printf("init game board\n");
    // draw_rect(p_frame_buffer, 0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 5, WHITE, 1);
    printf("init stick\n");
    draw_line(p_frame_buffer, 470, 154, STICK_WIDTH, STICK_HEIGHT, WHITE); // Stick
    printf("init ball\n");
    draw_line(p_frame_buffer, 465, 159, BALL_SIZE, BALL_SIZE, RED); //Ball
    // table
    printf("init brick\n");
    generate_brick(p_frame_buffer, 500);
}

int main(int argc, char *argv[])
{
    printf("Start breakout console\n");
    static uint16_t *p_frame_buffer = 0;
    int fbfd = open("/dev/fb0", O_RDWR);
    if (!fbfd) {
        printf("[Q_Q]Can't open the framebuffer\n");
        return -1;
    }
    printf("reading fixed information\n");
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
        printf("[Q_Q]Error reading fixed information.\n");
        return -1;
    }
    printf("reading variable information\n");
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
        printf("[Q_Q]Error: reading variable information.\n");
        return -1;
    }
    printf("Clearing Screen? ");
    if (argc > 1 ) {
        if(strcmp(argv[2], "-c")){
            printf("clear the screen\n");
            fill_screen(p_frame_buffer, BLACK, RESOLUTION);
            return 0;
        }
    }else {
        printf("No!\n");
    }
    FT_Face face = init_freetype();
    long screensize = finfo.line_length * vinfo.yres;
    printf("init frame buffer\n");
    p_frame_buffer = (uint16_t *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if (!p_frame_buffer) {
        printf("[Q_Q]Error: failed to map framebuffer device to memory.\n");
        return -1;
    }
    init_game_board(p_frame_buffer);

}