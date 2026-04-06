#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <video/mipi_display.h>
#include <linux/delay.h>

#include "fbtft.h"
#define DRVNAME		"fb_ili9488"
#define WIDTH		320
#define HEIGHT		480

/* this init sequence matches PiScreen */
static const s16 default_init_sequence[] = {
	/* Interface Mode Control */
	-1, 0xb0, 0x0,
	-1, MIPI_DCS_EXIT_SLEEP_MODE,
	-2, 250,
	/* Interface Pixel Format */
	-1, MIPI_DCS_SET_PIXEL_FORMAT, 0x66,
	/* Power Control 3 */
	-1, 0xC2, 0x44,
	/* VCOM Control 1 */
	-1, 0xC5, 0x00, 0x00, 0x00, 0x00,
	/* PGAMCTRL(Positive Gamma Control) */
	-1, 0xE0, 0x0F, 0x1F, 0x1C, 0x0C, 0x0F, 0x08, 0x48, 0x98,
		  0x37, 0x0A, 0x13, 0x04, 0x11, 0x0D, 0x00,
	/* NGAMCTRL(Negative Gamma Control) */
	-1, 0xE1, 0x0F, 0x32, 0x2E, 0x0B, 0x0D, 0x05, 0x47, 0x75,
		  0x37, 0x06, 0x10, 0x03, 0x24, 0x20, 0x00,
	/* Digital Gamma Control 1 */
	-1, 0xE2, 0x0F, 0x32, 0x2E, 0x0B, 0x0D, 0x05, 0x47, 0x75,
		  0x37, 0x06, 0x10, 0x03, 0x24, 0x20, 0x00,
	-1, MIPI_DCS_EXIT_SLEEP_MODE,
	-1, MIPI_DCS_SET_DISPLAY_ON,
	/* end marker */
	-3
};

static int init_display(struct fbtft_par *par)
{
    fbtft_par_dbg(DEBUG_INIT_DISPLAY, par, "%s()\n", __func__);

    printk("initializing ili9488\n");

    /* Interface Mode Control */
    write_reg(par, 0xB0, 0x00);

    /* Exit sleep */
    write_reg(par, MIPI_DCS_EXIT_SLEEP_MODE);
    mdelay(250);

    /* Interface Pixel Format: 18bit/pixel */
    write_reg(par, MIPI_DCS_SET_PIXEL_FORMAT, 0x66);

    /* Power Control 3 */
    write_reg(par, 0xC2, 0x44);

    /* VCOM Control 1 */
    write_reg(par, 0xC5, 0x00, 0x00, 0x00, 0x00);

    /* Positive Gamma Control */
    write_reg(par, 0xE0, 0x0F, 0x1F, 0x1C, 0x0C, 0x0F, 0x08,
                    0x48, 0x98, 0x37, 0x0A, 0x13, 0x04,
                    0x11, 0x0D, 0x00);

    /* Negative Gamma Control */
    write_reg(par, 0xE1, 0x0F, 0x32, 0x2E, 0x0B, 0x0D, 0x05,
                    0x47, 0x75, 0x37, 0x06, 0x10, 0x03,
                    0x24, 0x20, 0x00);

    /* Digital Gamma Control */
    write_reg(par, 0xE2, 0x0F, 0x32, 0x2E, 0x0B, 0x0D, 0x05,
                    0x47, 0x75, 0x37, 0x06, 0x10, 0x03,
                    0x24, 0x20, 0x00);

    /* Exit sleep again (some panels need it twice) */
    write_reg(par, MIPI_DCS_EXIT_SLEEP_MODE);
    mdelay(250);

    /* Display ON */
    write_reg(par, MIPI_DCS_SET_DISPLAY_ON);
	// printk("testing ili9488\n");
	// ili9488_test_colors(par);
    return 0;
}

static void set_addr_win(struct fbtft_par *par, int xs, int ys, int xe, int ye)
{
	write_reg(par, MIPI_DCS_SET_COLUMN_ADDRESS,
		  xs >> 8, xs & 0xFF, xe >> 8, xe & 0xFF);

	write_reg(par, MIPI_DCS_SET_PAGE_ADDRESS,
		  ys >> 8, ys & 0xFF, ye >> 8, ye & 0xFF);

	write_reg(par, MIPI_DCS_WRITE_MEMORY_START);
}

static int set_var(struct fbtft_par *par)
{
	switch (par->info->var.rotate) {
	case 0:
		write_reg(par, MIPI_DCS_SET_ADDRESS_MODE,
			  0x80 | (par->bgr << 3));
		break;
	case 90:
		write_reg(par, MIPI_DCS_SET_ADDRESS_MODE,
			  0x20 | (par->bgr << 3));
		break;
	case 180:
		write_reg(par, MIPI_DCS_SET_ADDRESS_MODE,
			  0x40 | (par->bgr << 3));
		break;
	case 270:
		write_reg(par, MIPI_DCS_SET_ADDRESS_MODE,
			  0xE0 | (par->bgr << 3));
		break;
	default:
		break;
	}

	return 0;
}

/* 16bpp converted to 18bpp stored in 24-bit over 8-bit databus */
int fbtft_write_vmem18to24_bus8(struct fbtft_par *par, size_t offset, size_t len)
{
	u16 *vmem16;
	u8 *txbuf = par->txbuf.buf;
	size_t remain;
	size_t to_copy;
	size_t tx_array_size;
	int i;
	int ret = 0;

	/* remaining number of pixels to send */
	remain = len / 2;
	vmem16 = (u16 *)(par->info->screen_buffer + offset);

	if ((int)(par->gpio.dc) != -1)
		gpiod_set_value(par->gpio.dc, 1);

	/* number of pixels that fits in the transmit buffer */
	tx_array_size = par->txbuf.len / 3;

	while (remain) {
		/* number of pixels to copy in one iteration of the loop */
		to_copy = min(tx_array_size, remain);
		dev_dbg(par->info->device, "    to_copy=%zu, remain=%zu\n",
						to_copy, remain - to_copy);

		for (i = 0; i < to_copy; i++) {
			u16 pixel = vmem16[i];
			u16 b = pixel & 0x1f;
			u16 g = (pixel & (0x3f << 5)) >> 5;
			u16 r = (pixel & (0x1f << 11)) >> 11;

			txbuf[i * 3 + 2] = (r & 0x1F) << 3;
			txbuf[i * 3 + 1] = (g & 0x3F) << 2;
			txbuf[i * 3 + 0] = (b & 0x1F) << 3;
		}

		vmem16 = vmem16 + to_copy;
		ret = par->fbtftops.write(par, par->txbuf.buf, to_copy * 3);
		if (ret < 0)
			return ret;
		remain -= to_copy;
	}

	return ret;
}
EXPORT_SYMBOL(fbtft_write_vmem18to24_bus8);

static struct fbtft_display display = {
	.regwidth = 8,
	.width = WIDTH,
	.height = HEIGHT,
	// .init_sequence = default_init_sequence,
	.fbtftops = {
		.init_display = init_display,
		.set_addr_win = set_addr_win,
		.set_var = set_var,
		.write_vmem = fbtft_write_vmem18to24_bus8,
	},
};

FBTFT_REGISTER_DRIVER(DRVNAME, "ilitek,ili9488", &display);

MODULE_ALIAS("spi:" DRVNAME);
MODULE_ALIAS("platform:" DRVNAME);
MODULE_ALIAS("spi:ili9488");
MODULE_ALIAS("platform:ili9488");

MODULE_DESCRIPTION("FB driver for the ILI9488 LCD Controller");
MODULE_AUTHOR("huaichun.yu0429@gmail.com");
MODULE_LICENSE("GPL");
