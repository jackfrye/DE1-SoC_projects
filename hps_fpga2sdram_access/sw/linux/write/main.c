#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <inttypes.h>

void *h2p_lw_virtual_base;
volatile unsigned int * lw_pio_ptr = NULL ;

#define FPGA_LW_BASE 	0xff200000
#define FPGA_LW_SPAN	0x00001000


int main(int argc, char *argv[]) {

    uint32_t byte_addr;
    uint32_t addr;
    uint32_t data;
    uint32_t page_base;
    uint32_t page_offset;

    if(argc < 3)
    {
        printf("Not enough arguments provided\nUsage ./write <addr> <data>\n");
        return 1;
    }
    else
    {
        if (sscanf (argv[1], "%xu", &byte_addr)!=1) {
            fprintf(stderr, "error - not a hex integer");
            printf("Exiting\n");
            return 1;
        }
        if(byte_addr % 4 != 0)
        {
            printf("Invalid address. Byte address must be multiple of 4\n");
            return 1;
        }
        addr = byte_addr / 4;

        if (sscanf (argv[2], "%xu", &data)!=1) {
            fprintf(stderr, "error - not a hex integer");
            printf("Exiting\n");
            return 1;
        }
    }

    page_base = FPGA_LW_BASE + ((byte_addr / FPGA_LW_SPAN)*FPGA_LW_SPAN);
    page_offset = (byte_addr % FPGA_LW_SPAN)/4;

    int fd = open("/dev/mem", ( O_RDWR | O_SYNC ) );
    h2p_lw_virtual_base = mmap(NULL, FPGA_LW_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, page_base);
    if (h2p_lw_virtual_base == MAP_FAILED) {
        perror("Can't map memory");
        return -1;
    }

    lw_pio_ptr = (unsigned int *)(h2p_lw_virtual_base);

    *(lw_pio_ptr+page_offset) = data;

    close(fd);

    return 0;
}
