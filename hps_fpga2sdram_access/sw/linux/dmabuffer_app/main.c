
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include "dma_buffer.h"

#define WINDOW_SIZE            0x1000
#define FPGA_LW_BASE 	       0xFF200000
#define FPGA_LW_SPAN	       0x00001000
#define ADDR_SPAN_WINDOW_BASE  0x00010000
#define ADDR_SPAN_CTRL_BASE    0x00020000

void *h2p_lw_virtual_base;
volatile unsigned int * lw_pio_ptr = NULL ;
uint32_t phys_addr = 0;
static dma_buffer_interface_t *buffer_intf;

int read_buffer_fpga()
{
    int fd;
    int i;
    int data;
    int window;

    ///////////////////////////////////// Program Window Index ////////////////////////////////////////////////////
    fd = open("/dev/mem", ( O_RDWR | O_SYNC ) );
    h2p_lw_virtual_base = mmap(NULL, FPGA_LW_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, (FPGA_LW_BASE+ADDR_SPAN_CTRL_BASE));
    if (h2p_lw_virtual_base == MAP_FAILED) {
        perror("Can't map memory");
        return -1;
    }
    lw_pio_ptr = (unsigned int *)(h2p_lw_virtual_base);
    // Write window number to address space extender
    *(lw_pio_ptr) = phys_addr;
    close(fd);
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////// Read 4K Worth Data through Address Span Extender //////////////////////////////
    fd = open("/dev/mem", ( O_RDWR | O_SYNC ) );
    h2p_lw_virtual_base = mmap(NULL, FPGA_LW_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, (FPGA_LW_BASE+ADDR_SPAN_WINDOW_BASE));
    if (h2p_lw_virtual_base == MAP_FAILED) {
        perror("Can't map memory");
        return -1;
    }
    lw_pio_ptr = (unsigned int *)(h2p_lw_virtual_base);
    for(i = 0; i < BUFFER_SIZE/((sizeof(volatile unsigned int)/sizeof(unsigned char))); i++)
    {
        data = *(lw_pio_ptr+i);
        printf("%d: %x\n", i, data);
    }
    close(fd);
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////

    return 0;

}

int read_buffer_os()
{
    int fd;
    int i;

    fd = open("/dev/dmabuffer", O_RDWR);
    if (fd < 1) {
        printf("Unable to open device file");
        exit(EXIT_FAILURE);
    }

    buffer_intf = (dma_buffer_interface_t *)mmap(NULL, sizeof(dma_buffer_interface_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    for(i = 0; i < BUFFER_SIZE; i++)
    {
        printf("%d: %x\n", i, buffer_intf->buffer[i]);
    }

    close(fd);

    return 0;
}

int write_buffer_os()
{
    int fd;
    int i;

    fd = open("/dev/dmabuffer", O_RDWR);
    if (fd < 1) {
        printf("Unable to open device file");
        exit(EXIT_FAILURE);
    }

    buffer_intf = (dma_buffer_interface_t *)mmap(NULL, sizeof(dma_buffer_interface_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    for(i = 0; i < BUFFER_SIZE; i++)
    {
        buffer_intf->buffer[i] = i;
    }

    close(fd);
}

int main(int argc, char **argv)
{
    int fd;
    uint32_t phys_addr_calc;
    uint32_t window;
    int i;
    int dummy;


    printf("Opening\n");
    fd = open("/dev/dmabuffer", O_RDWR);
    if (fd < 1) {
        printf("Unable to open device file");
        exit(EXIT_FAILURE);
    }
    printf("mmap\n");
    buffer_intf = (dma_buffer_interface_t *)mmap(NULL, sizeof(dma_buffer_interface_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    phys_addr = buffer_intf->phys_addr;
    printf("physical address: %x\n", phys_addr);
    close(fd);

    printf("Read buffer from fpga\n");
    read_buffer_fpga();
    printf("Read buffer from OS\n");
    read_buffer_os();
    printf("Write buffer from OS\n");
    write_buffer_os();
    printf("Read buffer from fpga\n");
    read_buffer_fpga();

}
