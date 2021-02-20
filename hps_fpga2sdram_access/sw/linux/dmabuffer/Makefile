obj-m := dmabuffer.o

#KDIR := /lib/modules/$(shell uname -r)/build
KDIR := /home/jfrye/Documents/FPGA/intel/cyclone5/projects/v4l2_soc/sw/linux/linux-socfpga.a9

PWD := $(shell pwd)

CC := /home/jfrye/Documents/FPGA/intel/cyclone5/sw/toolchains/gcc-linaro-7.4.1-2019.02-x86_64_arm-eabi/bin/arm-eabi-gcc

default:
	$(MAKE) ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -C $(KDIR) M=$(PWD) modules 

clean:
	rm modules.order Module.symvers dmabuffer.ko dmabuffer.mod dmabuffer.mod.c dmabuffer.mod.o dmabuffer.o
