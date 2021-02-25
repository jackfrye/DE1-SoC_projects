obj-m := altera_dma_buffer_driver.o

#KDIR := /lib/modules/$(shell uname -r)/build
KDIR := /home/jfrye/Documents/FPGA/intel/cyclone5/projects/v4l2_soc/sw/linux/linux-socfpga.a9
INC := /home/jfrye/Documents/FPGA/intel/cyclone5/sw/toolchains/gcc-linaro-7.4.1-2019.02-x86_64_arm-eabi/lib/gcc/arm-eabi/7.4.1/include
PWD := $(shell pwd)

CC := /home/jfrye/Documents/FPGA/intel/cyclone5/sw/toolchains/gcc-linaro-7.4.1-2019.02-x86_64_arm-eabi/bin/arm-eabi-gcc

default:
	$(MAKE) ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -C $(KDIR) M=$(PWD) modules -I$(INC)

clean:
	rm modules.order Module.symvers altera_dma_buffer_driver.ko altera_dma_buffer_driver.mod altera_dma_buffer_driver.mod.c altera_dma_buffer_driver.mod.o altera_dma_buffer_driver.o
