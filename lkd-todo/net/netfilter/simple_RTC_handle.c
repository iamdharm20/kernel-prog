

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/vmalloc.h>
#include <asm/io.h>
#include <linux/proc_fs.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
 
MODULE_LICENSE("Dual BSD/GPL");
 
static int irq = 12;
 
module_param(irq,int,0);
 
int * dev_id;
int flag = 0;
volatile int *rtc;
 
static irqreturn_t *irq_handle(void * dev_id)
{
    printk(KERN_DEBUG "Interrupt\n");
    rtc[7] = 0x1;
    return 0;
}
 
static int acme_init (void)
{
    int req_irq = request_irq(irq, &irq_handle, 0, "mydev", &dev_id);
    rtc=ioremap(0x101e8000,0x1000);
    return 0;
}
 
static void hello_cleanup (void)
{
    printk (KERN_DEBUG "Module unloaded succefully.\n");
    free_irq(irq, &dev_id);
}
 
module_init (acme_init);
module_exit (hello_cleanup);


