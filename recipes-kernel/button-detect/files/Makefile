obj-m += button-detect.o

KDIR ?= $(KERNEL_SRC)
PWD := $(CURDIR)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean