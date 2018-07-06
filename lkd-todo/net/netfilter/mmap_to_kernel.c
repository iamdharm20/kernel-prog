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
#include <asm/types.h>
#include <asm/io.h>
#include <asm/dma-mapping.h>
 
static void *buffer;
static int buf_size=8192;
static int counter=0;
 
static irqreturn_t *irq_handle(int irq,void * dev_id,struct pt_regs *regs)
{
    counter++;
    sprintf(buffer,"count=%d",counter);
    return 0;
}
 
static int device_mmap(struct file *file,struct vm_area_struct* vma)
{
    int size;
    size=vma->vm_end - vma->vm_start;
    if(remap_pfn_range(vma,vma->vm_start,virt_to_phys(buffer)>>PAGE_SHIFT,buf_size,vma->vm_page_prot))
        return -EAGAIN;
    return 0;
}
 
static struct file_operations acme_fops =
{
    .owner = THIS_MODULE,
    .mmap = device_mmap,
};
 
static int acme_count = 1;
static dev_t acme_dev;
 
static struct cdev *acme_cdev;
 
//static struct device *dev;
 
static int
hello_init (void)
{
//    dma_addr_t dm;
    request_irq(12, &irq_handle, IRQF_SHARED, "mydev", &hello_init);
    buffer = kzalloc(8192,GFP_USER);
//    buffer = dma_alloc_coherent(NULL,8092,&dm,0);
    buf_size = 8192;
    if(!buffer)
    {
         printk (KERN_INFO "kzalloc error.\n");
         return -1;
    }
    if(alloc_chrdev_region(&acme_dev,0,acme_count,"acme1"))
    {
         printk (KERN_INFO "alloc chrdev error.\n");
         return -1;
    }
 
    acme_cdev=cdev_alloc();
    if(!acme_cdev)
    {
        printk (KERN_INFO "cdev alloc error.\n");
         return -1;
    }
    acme_cdev->ops = &acme_fops;
    acme_cdev->owner = THIS_MODULE;
 
    if(cdev_add(acme_cdev,acme_dev,acme_count))
    {
        printk (KERN_INFO "cdev add error.\n");
         return -1;
    }
 
  return 0;
 
}
 
static void
hello_cleanup (void)
{
 
    printk (KERN_INFO "hello unloaded succefully.\n");
    free_irq(12, &hello_init);
 
}
 
module_init (hello_init);
module_exit (hello_cleanup);
MODULE_LICENSE("GPL");
