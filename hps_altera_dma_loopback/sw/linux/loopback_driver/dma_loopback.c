#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/cdev.h>
#include <linux/version.h>
#include <linux/delay.h>
#include <linux/dmaengine.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>
#include <linux/of_dma.h>
#include "dma_buffer.h"

#define AXI_LW_BASE     0xFF200000
#define DMA_BASE        0x20
#define DMA_REGS_SIZE   0x8
#define STAT_OFF        0x0
#define READ_ADDR_OFF   0x1
#define WRITE_ADDR_OFF  0x2
#define LEN_OFF         0x3
#define CONTROL_OFF     0x6

#define BUFFER_SIZE 0x1000

#define TX_MAJOR       42
#define TX_MAX_MINORS  5

#define RX_MAJOR       43
#define RX_MAX_MINORS  5

int dev_major = 0;

typedef struct dma_buf {
    volatile phys_addr_t phys_addr;
    volatile unsigned int *virt_addr;
    volatile unsigned int *dma_regs;
    dma_addr_t dma_handle;
    struct device *dev;
    dev_t dev_node;
    struct cdev cdev;
    struct class *class;
    dma_buffer_interface_t *interface;
} buf_t;

buf_t buffers[2];

static int open(struct inode *inode, struct file *file)
{
   
    file->private_data = container_of(inode->i_cdev, buf_t, cdev);

    return 0;
    
}

static long ioctl(struct file *file, unsigned int unused , unsigned long arg)
{
    volatile unsigned int *dma_regs_handle;
    struct timespec start;
    struct timespec now;
    int elapsed = 0;

    buf_t *tx_buf = (buf_t *)file->private_data;

    // Program the DMA and perform the transfer
    dma_regs_handle = tx_buf->dma_regs;

    // ReadAddress
    *(dma_regs_handle+READ_ADDR_OFF) = (unsigned int)(tx_buf->interface->phys_addr_tx );

    // WriteAddress
    *(dma_regs_handle+WRITE_ADDR_OFF) = (unsigned int)(tx_buf->interface->phys_addr_rx );

    // Length
    *(dma_regs_handle+LEN_OFF) = (unsigned int)(BUFFER_SIZE / (sizeof(uint32_t) / sizeof(unsigned char)) );

    printk("Read addr: %x\n", *(dma_regs_handle+READ_ADDR_OFF) );
    printk("Write addr: %x\n", *(dma_regs_handle+WRITE_ADDR_OFF) );
    printk("Length: %x\n", *(dma_regs_handle+LEN_OFF) );
    printk("Control: %x\n", *(dma_regs_handle+CONTROL_OFF) );

    getnstimeofday(&start);
    // Word XFER and Start
    *(dma_regs_handle+CONTROL_OFF) = 0x0000008C; 


    while( !(0x00000001 & *(dma_regs_handle+STAT_OFF) ) & (elapsed < 3) )
    {
        getnstimeofday(&now);
        elapsed = (now.tv_sec-start.tv_sec);
    }

    if(elapsed >= 3)
    {
        printk(KERN_INFO "Timeout!!!\n");
    }

    return 0;

}

static int mmap(struct file *file_p, struct vm_area_struct *vma)
{
	buf_t *buf_local = (buf_t *)file_p->private_data;

	return dma_mmap_coherent(buf_local->dev, 
                                 vma,
				 buf_local->interface, buf_local->dma_handle,
				 vma->vm_end - vma->vm_start
        );

}

static int dmadev_uevent(struct device *dev, struct kobj_uevent_env *env)
{
    add_uevent_var(env, "DEVMODE=%#o", 0666);
    return 0;
}

const struct file_operations fops_rx = {
    .owner           = THIS_MODULE,
    .open            = open,
    .mmap            = mmap
};

const struct file_operations fops_tx = {
    .owner           = THIS_MODULE,
    .open            = open,
    .mmap            = mmap,
    .unlocked_ioctl  = ioctl
};

static int dmabuffer_probe(struct platform_device *pdev)
{
    int err;

    printk(KERN_INFO "dmabuffer module initialized\n");

    buffers[0].dev = &pdev->dev;
    buffers[0].interface = (dma_buffer_interface_t*)dma_alloc_coherent(buffers[0].dev, 
                                                                        sizeof(dma_buffer_interface_t), 
                                                                        &buffers[0].dma_handle, 
                                                                        GFP_KERNEL);

    buffers[0].interface->phys_addr_tx = (uint32_t)buffers[0].dma_handle;

    err = alloc_chrdev_region(&buffers[0].dev_node,  0, 1, "dmabuffer_tx");
    dev_major = MAJOR(buffers[0].dev_node);
    if (err != 0) 
    {
        printk(KERN_INFO, "Error registering buff\n");
        return err;
    }
    
    printk( KERN_INFO "Init TX device\n");

    cdev_init(&(buffers[0].cdev), &fops_tx);
    buffers[0].cdev.owner = THIS_MODULE;
    buffers[0].cdev.ops = &fops_tx;
    buffers[0].class = class_create(THIS_MODULE, "dmabuffer_tx");
    buffers[0].class->dev_uevent = dmadev_uevent;
    err = cdev_add(&(buffers[0].cdev), buffers[0].dev_node, 1);
    if(err < 0)
    {
        printk( KERN_INFO "Failed to add cdev for TX\n");
    }
    device_create(buffers[0].class, NULL, buffers[0].dev_node, NULL, "dmabuffer_tx");

    buffers[1].dev = &pdev->dev;
    buffers[1].interface = (dma_buffer_interface_t*)dma_alloc_coherent(buffers[1].dev, 
                                                                        sizeof(dma_buffer_interface_t), 
                                                                        &buffers[1].dma_handle, 
                                                                        GFP_KERNEL);
    buffers[1].interface->phys_addr_tx = (uint32_t)buffers[0].dma_handle;
    buffers[1].interface->phys_addr_rx = (uint32_t)buffers[1].dma_handle;
    buffers[0].interface->phys_addr_rx = (uint32_t)buffers[1].dma_handle;

    printk( KERN_INFO "buf[0] tx_phys_addr: %x\n", buffers[0].interface->phys_addr_tx);
    printk( KERN_INFO "buf[0] rx_phys_addr: %x\n", buffers[0].interface->phys_addr_rx);
    printk( KERN_INFO "buf[1] tx_phys_addr: %x\n", buffers[1].interface->phys_addr_tx);
    printk( KERN_INFO "buf[1] rx_phys_addr: %x\n", buffers[1].interface->phys_addr_rx);

    err = alloc_chrdev_region(&buffers[1].dev_node,  0, 1, "dmabuffer_rx");
    dev_major = MAJOR(buffers[1].dev_node);
    if (err != 0) 
    {
        printk(KERN_INFO, "Error registering buff\n");
        return err;
    }
    
    printk( KERN_INFO "Init RX device\n");

    cdev_init(&(buffers[1].cdev), &fops_rx);
    buffers[1].cdev.owner = THIS_MODULE;
    buffers[1].cdev.ops = &fops_rx;
    buffers[1].class = class_create(THIS_MODULE, "dmabuffer_rx");
    buffers[1].class->dev_uevent = dmadev_uevent;
    err = cdev_add(&(buffers[1].cdev), buffers[1].dev_node, 1);
    if(err < 0)
    {
        printk( KERN_INFO "Failed to add cdev for TX\n");
    }
    device_create(buffers[1].class, NULL, buffers[1].dev_node, NULL, "dmabuffer_rx");
    
    buffers[0].dma_regs = (unsigned int *)ioremap((unsigned long)(AXI_LW_BASE+DMA_BASE), (unsigned long)DMA_REGS_SIZE);
    

    return 0;
}

static int dmabuffer_remove(struct platform_device *pdev)
{

	printk(KERN_INFO "dma_proxy module exited\n");

	return 0;
}


static const struct of_device_id dmabuffers_altera_dma_of_ids[] = {
	{ .compatible = "jfrye,dma-buffers-altera-dma",},
	{}
};

static struct platform_driver dmabuffers_altera_dma_driver = {
	.driver = {
		.name = "dmabuffers-altera-dma",
		.owner = THIS_MODULE,
		.of_match_table = dmabuffers_altera_dma_of_ids,
	},
	.probe = dmabuffer_probe,
	.remove = dmabuffer_remove,
};

static int __init dmabuffer_init(void)
{
	return platform_driver_register(&dmabuffers_altera_dma_driver);

}

static void __exit dmabuffer_exit(void)
{
	platform_driver_unregister(&dmabuffers_altera_dma_driver);
}

module_init(dmabuffer_init)
module_exit(dmabuffer_exit)

MODULE_AUTHOR("Jack Frye");
MODULE_DESCRIPTION("Create Transmit and Receive Buffers and Control Altera DMA IP");
MODULE_LICENSE("GPL v2");
