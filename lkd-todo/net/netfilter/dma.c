

#include <linux/module.h>
#include <linux/kernel.h>    /* printk() */
#include <linux/moduleparam.h>
#include <asm/uaccess.h>
#include <asm/pgtable.h>
#include <linux/fs.h>
#include <linux/gfp.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/mm.h>
#include <linux/kdev_t.h>
#include <linux/proc_fs.h>
#include <linux/ioctl.h>
#include <linux/slab.h>
#include <linux/mempool.h>
#include <linux/mm.h>
#include <linux/device.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>
#include <asm/types.h>
#include <asm/io.h>
#include <asm/dma-mapping.h>
 
#define DEST_ADDRESS 0x73800000
#define SRC_ADDRESS   0x10400000
 
static volatile struct completion comp1;
static volatile struct dma_chan *dma_chan1;
 
static int device_mmap(struct file *file,struct vm_area_struct* vma)
{
    int size;
    printk("mmap fpga\n");
    size=vma->vm_end - vma->vm_start;
    if(remap_pfn_range(vma,vma->vm_start,DEST_ADDRESS>>PAGE_SHIFT,0x800000,vma->vm_page_prot))
        return -EAGAIN;
    return 0;
}
static void dma_complete_func(void *completion)
{
    complete(completion);
}
 
struct dma_async_tx_descriptor *tx1 = NULL;
 
static int device_ioctl(struct inode *inode,struct file *file,int num,int param)
{
        struct dma_device *dma_dev;
        enum dma_ctrl_flags flags;
        dma_cookie_t cookie;
        dma_dev = dma_chan1->device;
        flags =  DMA_CTRL_ACK | DMA_COMPL_SKIP_DEST_UNMAP | DMA_COMPL_SKIP_SRC_UNMAP;
        tx1 = dma_dev->device_prep_dma_memcpy(dma_chan1, DEST_ADDRESS,
                    SRC_ADDRESS+0x40000*param, 0x40000, flags);
        if (!tx1) {
            printk("Failed to prepare DMA memcpy\n");
            return -1;
        }
        init_completion(&comp1);
        tx1->callback = dma_complete_func;
        tx1->callback_param = &comp1;
 
        cookie = tx1->tx_submit(tx1);
        if (dma_submit_error(cookie)) {
            printk("Failed to do DMA tx_submit\n");
            return -1;
        }
 
        dma_async_issue_pending(dma_chan1);
 
        wait_for_completion(&comp1);
}
 
static struct file_operations fpga_fops =
{
    .owner = THIS_MODULE,
    .mmap = device_mmap,
    .ioctl = device_ioctl,
};
 
static int __init hello_init (void)
{
    dev_t fpga_dev;
    struct cdev *fpga_cdev;
    struct device *dev;
    dma_addr_t dm;
    dma_cap_mask_t mask;
 
    dma_cap_zero(mask);
    dma_cap_set(DMA_MEMCPY,mask);
    dma_chan1 = dma_request_channel(mask,0,NULL);
 
    if(dma_chan1 == 0)
    {
        printk("fpga:failed to request DMA channel\n");
    }
 
    if(register_chrdev_region(MKDEV(200,0),1,"fpga"))
    {
         printk (KERN_INFO "alloc chrdev error.\n");
         return -1;
    }
 
    fpga_cdev=cdev_alloc();
    if(!fpga_cdev)
    {
        printk (KERN_INFO "cdev alloc error.\n");
         return -1;
    }
    fpga_cdev->ops = &fpga_fops;
    fpga_cdev->owner = THIS_MODULE;
 
    if(cdev_add(fpga_cdev,MKDEV(200,0),1))
    {
        printk (KERN_INFO "cdev add error.\n");
         return -1;
    }
    printk("fpga driver loaded\n");
 
  return 0;
 
}
 
late_initcall(hello_init);
MODULE_LICENSE("GPL");


