
GCC=/home/jfrye/Documents/FPGA/intel/cyclone5/sw/toolchains/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc

all:
	$(GCC) -o loopback_app main.c -I/home/jfrye/Documents/FPGA/intel/cyclone5/projects/hps_v4l2/sw/linux/linux-socfpga.a9/include -I../loopback_driver


clean:
	rm loopback_app
