#include "TFT_tester.h"

void fill_screen(uint16_t *fb_men, int color, int pix_size)
{	
	for(int i=0;i<pix_size;i++)
	{
		*fb_men = color;
		fb_men ++;
	}
}

char RGB888_to_RGB565(uint8_t r, uint8_t g, uint8_t b){
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

bmp_info TFT_get_detail_info(uint8_t *buf)
{
    bmp_info bmp;
    bmp.type = (buf[1] << 8) + buf[0];
    bmp.size = (buf[2] + (buf[3] << 8)) + ((buf[4]<< 16) + (buf[5] << 24));
    bmp.width = buf[18] + (buf[19]<<8) + (buf[20]<<16) + (buf[21]<<24);
    bmp.height = buf[22] + (buf[23]<<8) + (buf[24]<<16) + (buf[25]<<24); //0x(buf[25])(buf[24])(buf[23])(buf[22]) 
    bmp.bitcount =  buf[28] + (buf[29]<<8);
    return bmp;
}

int show_bmp(char *file, uint16_t *frame_buffer)
{
    bmp_info bmp;
    char header[54];
    int header_size = 54;
    int bmp_fd = open(file,O_RDWR);
    if(bmp_fd < 0){
        printf("[Q_Q] bmp file open fail\n");
        return -1;
    }
    if(read(bmp_fd,header,header_size) <0)
    {
        printf("read file \"%s\" fail\n",file);
        return -1;	
    }
    bmp = TFT_get_detail_info(header);
    printf("%s's msg:\n",file);
    printf("\ttype:%2x\n",bmp.type);
    printf("\tsize:%d\n",bmp.size);
    printf("\tWidth:%d\n",bmp.width);
    printf("\tHeight:%d\n",bmp.height);
    printf("\tBitCount:%d\n",bmp.bitcount);
    if(bmp.type != 0x4d42)
    {
        printf("The picture is not bmp file\n");
        return -1;
    }

    if(bmp.bitcount != 24)
    {
        printf("Just support 24bit bmp file\n");
        return -1;
    }
    

    // skip bmp header
    int offset = header[10] | header[11]<<8 | header[12]<<16 | header[13]<<24;
    lseek(bmp_fd,offset,SEEK_SET);
    int width_size = bmp.width * 3; // 225 * 3(RGB)
    int width_size_padded = width_size;
    if(width_size % 4 != 0){
        width_size_padded +=  4 - (width_size % 4);
        printf("Need padding, non padded = %d padded = %d !!!\n",width_size,width_size_padded);
    }else{
        width_size_padded =  width_size;
        printf("No need to pad!!! %d \n",width_size);
    }
    printf("read %d col\n",bmp.height);
    char *col_data = malloc(width_size_padded);
    for(int i = 0; i<bmp.height;i++){
        int ret = read(bmp_fd,col_data,width_size_padded);
        if(ret == -1){
            printf("read return code: %d\n",ret);
            perror("read");
            return -1;
        }
        if(ret != width_size_padded){
            printf("Reading size mismatch ret: %d != %d\n",ret,width_size_padded);
            return -1;
        }
        // printf("ret value: %d\n",ret);
        
        for (int x = 0; x < bmp.width; x++) {
            int idx = x * 3; // each pixel have RGB(need to read three byte)
            uint8_t blue  = (col_data[idx]   >> 3) & 0x1F;
            uint8_t green = (col_data[idx+1] >> 2) & 0x3F;
            uint8_t red   = (col_data[idx+2] >> 3) & 0x1F;

            uint16_t pixel = (red << 11) | (green << 5) | blue;
            // printf("[%d,%d] Blue: %d, Green: %d, Red: %d, Pixel value: %d\n",i,x,blue,green,red,pixel);
            *frame_buffer = pixel;
            frame_buffer++;
        }
        // printf("offset %d\n",SCREEN_WIDTH - bmp.width);
        if(bmp.width < SCREEN_WIDTH){
            frame_buffer += (SCREEN_WIDTH - bmp.width) ;
        }
        
    }
    
	return 0;
}

int draw_char_fb(FT_Face face, uint16_t *fb, int screen_w, int screen_h,
                 int x, int y, char c, uint16_t color)
{
    printf("\n Loading %c\n",c);
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
                    fb[fb_y * screen_w + fb_x] = color;
                }
            }
        }
    }
    return 0;
}

int draw_text_fb(FT_Face face, uint16_t *fb, int screen_w, int screen_h,
                 int x, int y, const char *str, uint16_t color)
{
    int pen_x = x;
    int pen_y = y;

    for (const char *p = str; *p; p++) {
        printf("%c ",p);
        if (draw_char_fb(face, fb, screen_w, screen_h, pen_x, pen_y, *p, color) == -1) {
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


int main(int argc, char *argv[])
{
    printf("%d %s\n",argc,argv[0]);
    printf("open Frame Buffer\n");
    static uint16_t *p_frame_buffer = 0;
    int fbfd = open("/dev/fb0",O_RDWR);
    if(!fbfd){
        printf("[Q_Q]Can't open the framebuffer\n");
        return -1;
    }

    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo))
    {
        printf("[Q_Q]Error reading fixed information.\n");
        return -1;
    }

	if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo))
	{
		printf("[Q_Q]Error: reading variable information.\n");
		return -1;
	}

    FT_Face face = init_freetype();
    long screensize = finfo.line_length * vinfo.yres;
    printf("screen size %d line length %d yres %d \n",screensize,finfo.line_length,vinfo.yres);
    p_frame_buffer = (uint16_t *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if (p_frame_buffer == -1)
    {
        printf("[Q_Q]Error: failed to map framebuffer device to memory.\n");
        return -1;
    }

    if(argc > 2 && strcmp(argv[1],"-t") == 0){
        if(strcmp(argv[2],"r") == 0){
            printf("Fill red\n");
            fill_screen(p_frame_buffer,RED,RESOLUTION);
        }else if(strcmp(argv[2],"g") == 0){
            printf("Fill green\n");
	        fill_screen(p_frame_buffer,GREEN,RESOLUTION);
        }else if(strcmp(argv[2],"b") == 0){
            printf("Fill blue\n");
            fill_screen(p_frame_buffer,BLUE,RESOLUTION);
        }
    }else{
        printf("Fill black\n");
        fill_screen(p_frame_buffer,BLACK,RESOLUTION);  
    }

    if(argc > 2 && strcmp(argv[1],"-p") == 0){
        printf("Show bmp\n");
        show_bmp(argv[2], p_frame_buffer);
    }

    if(argc > 2 && strcmp(argv[1],"-f") == 0){
        printf("Show fonts %s\n",argv[2]);
        draw_text_fb(face, p_frame_buffer, SCREEN_WIDTH, SCREEN_HEIGHT, 50, 50, argv[2], 0xFFFF);
    }

    close(fbfd);
}