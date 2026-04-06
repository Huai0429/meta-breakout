obj-$(CONFIG_FB_ILI9488) += fb_ili9488.o

CONFIG_FB_ILI9488 ?= m
SRC := $(shell pwd)
ccflags-y += -I$(KERNEL_SRC)/drivers/staging/fbtft


all:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) CONFIG_FB_ILI9488=$(CONFIG_FB_ILI9488) modules

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) modules_install

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) clean