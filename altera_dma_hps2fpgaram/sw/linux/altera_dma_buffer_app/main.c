#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include "dma_buffer.h"

#define FPGA_LW_BASE 	       0xFF200000
#define FPGA_LW_SPAN	       0x00001000

static dma_buffer_interface_t *buffer_intf_tx;
void *h2p_lw_virtual_base;
volatile unsigned int * lw_pio_ptr = NULL ;
uint32_t phys_addr = 0;

int read_tx_buf()
{
    int fd;
    int i;

    fd = open("/dev/alt_dmabuffer", O_RDWR);
    if (fd < 1) {
        printf("Unable to open device file");
        exit(EXIT_FAILURE);
    }
    buffer_intf_tx = (dma_buffer_interface_t *)mmap(NULL, sizeof(dma_buffer_interface_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    for(i = 0; i < BUFFER_SIZE; i++)
    {
        printf("%d: %x\n", i, buffer_intf_tx->buffer[i]);
    }

    close(fd);
}

int write_tx_buf()
{
    int fd;
    int i;

    fd = open("/dev/alt_dmabuffer", O_RDWR);
    if (fd < 1) {
        printf("Unable to open device file");
        exit(EXIT_FAILURE);
    }
    buffer_intf_tx = (dma_buffer_interface_t *)mmap(NULL, sizeof(dma_buffer_interface_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    for(i = 0; i < BUFFER_SIZE; i++)
    {
        buffer_intf_tx->buffer[i] = i;
    }

    close(fd);
}

int read_fpga_ram()
{

    int fd;
    int i;
    int data;

    fd = open("/dev/mem", ( O_RDWR | O_SYNC ) );
    h2p_lw_virtual_base = mmap(NULL, FPGA_LW_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, (FPGA_LW_BASE+BRAM_BASE));
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
}

int dma_xfer()
{
    int fd;
    int dummy;
    fd = open("/dev/alt_dmabuffer", O_RDWR);
    if (fd < 1) {
        printf("Unable to open device file");
        exit(EXIT_FAILURE);
    }
    ioctl(fd, 0, &dummy);
    close(fd);
}

int main(int argc, char *argv[])
{
    int err;

    printf("Reading TX buff\n");
    err = read_tx_buf();
    printf("Writing TX buff\n");
    err = write_tx_buf();
    printf("Reading TX buff\n");
    err = read_tx_buf();
    printf("Read FPGA RAM\n");
    read_fpga_ram();
    printf("Peforming DMA Xfer\n");
    dma_xfer();
    printf("Read FPGA RAM\n");
    read_fpga_ram();

    return 0;

}
