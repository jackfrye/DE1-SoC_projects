
#define BUFFER_SIZE 0x1000
#define BRAM_BASE   0x40000

typedef struct dma_buffer_interface
{
    unsigned char buffer[BUFFER_SIZE];
    uint32_t phys_addr_tx;
    uint32_t phys_addr_rx;
} dma_buffer_interface_t;
