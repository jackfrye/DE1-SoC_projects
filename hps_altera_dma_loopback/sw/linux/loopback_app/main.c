#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include "dma_buffer.h"

static dma_buffer_interface_t *buffer_intf_tx;
static dma_buffer_interface_t *buffer_intf_rx;

int read_tx_buf()
{
    int fd;
    int i;

    fd = open("/dev/dmabuffer_tx", O_RDWR);
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

int read_rx_buf()
{
    int fd;
    int i;

    fd = open("/dev/dmabuffer_rx", O_RDWR);
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

    fd = open("/dev/dmabuffer_tx", O_RDWR);
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

int dma_xfer()
{
    int fd;
    int dummy;
    fd = open("/dev/dmabuffer_tx", O_RDWR);
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

    //printf("Reading TX buff\n");
    //err = read_tx_buf();
    //printf("Reading RX buff\n");
    //err = read_rx_buf();
    printf("Writing TX buff\n");
    write_tx_buf();
    //printf("Reading TX buff\n");
    //err = read_tx_buf();
    printf("Doing DMA XFER\n");
    err = dma_xfer();
    //printf("Reading RX buff\n");
    //err = read_rx_buf();

    return 0;
}
