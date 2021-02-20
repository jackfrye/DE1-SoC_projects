#include <linux/dmaengine.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>
#include <linux/of_dma.h>
#include <linux/delay.h>
#include "dma_buffer.h"

#define TX_MAJOR       42
#define TX_MAX_MINORS  5

int dev_major = 0;

typedef struct tx_dma_buf {
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

buf_t *buf;

static int open(struct inode *inode, struct file *file)
{
   
    file->private_data = container_of(inode->i_cdev, buf_t, cdev);

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


const struct file_operations fops = {
    .owner           = THIS_MODULE,
    .open            = open,
    .mmap            = mmap
};

static int dmabuffer_probe(struct platform_device *pdev)
{
    int err;

    printk(KERN_INFO "dmabuffer module initialized\n");

    buf = (buf_t*)kmalloc(sizeof(buf_t), GFP_KERNEL);
    buf->dev = &pdev->dev;
    buf->interface = (dma_buffer_interface_t*)dma_alloc_coherent(buf->dev, sizeof(dma_buffer_interface_t), &buf->dma_handle, GFP_KERNEL);
    buf->interface->phys_addr = (uint32_t)buf->dma_handle;
    printk( KERN_INFO "phys_addr: %x\n", buf->interface->phys_addr);

    err = alloc_chrdev_region(&buf->dev_node,  0, 1, "dmabuffer");
    dev_major = MAJOR(buf->dev_node);
    if (err != 0) 
    {
        printk(KERN_INFO, "Error registering buff\n");
        return err;
    }
    
    printk( KERN_INFO "Init device\n");

    cdev_init(&(buf->cdev), &fops);
    buf->cdev.owner = THIS_MODULE;
    buf->cdev.ops = &fops;
    buf->class = class_create(THIS_MODULE, "dmabuffer");
    buf->class->dev_uevent = dmadev_uevent;
    err = cdev_add(&(buf->cdev), buf->dev_node, 1);
    if(err < 0)
    {
        printk( KERN_INFO "Failed to add cdev for TX\n");
    }
    device_create(buf->class, NULL, buf->dev_node, NULL, "dmabuffer");

    printk( KERN_INFO "End init device\n");

    return 0;
}

static int dmabuffer_remove(struct platform_device *pdev)
{

	printk(KERN_INFO "dma_proxy module exited\n");

	return 0;
}


static const struct of_device_id dmabuffer_of_ids[] = {
	{ .compatible = "jfrye,dma-buffer",},
	{}
};

static struct platform_driver dmabuffer_driver = {
	.driver = {
		.name = "dmabuffer",
		.owner = THIS_MODULE,
		.of_match_table = dmabuffer_of_ids,
	},
	.probe = dmabuffer_probe,
	.remove = dmabuffer_remove,
};

static int __init dmabuffer_init(void)
{
	return platform_driver_register(&dmabuffer_driver);

}

static void __exit dmabuffer_exit(void)
{
	platform_driver_unregister(&dmabuffer_driver);
}

module_init(dmabuffer_init)
module_exit(dmabuffer_exit)

MODULE_AUTHOR("Jack Frye");
MODULE_DESCRIPTION("Get DMA buffer for Cyclone5");
MODULE_LICENSE("GPL v2");
