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
#include <linux/poll.h>
 
static void *buffer;
static int buf_size;
int data_avail_to_read=0,data_avail_to_write=0;
DECLARE_WAIT_QUEUE_HEAD(hr);
DECLARE_WAIT_QUEUE_HEAD(hw);
 
static ssize_t
acme_read(struct file *file, char __user *buf,size_t count,loff_t *ppos)
{
     int remaining_bytes;
   /* Number of bytes left to read in the open file */
   remaining_bytes = min(buf_size - (int) (*ppos), (int) count);
 
   if (remaining_bytes == 0) {
      /* All read, returning 0 (End Of File) */
      return 0;
   }
 
   if (copy_to_user(buf /* to */, *ppos+buffer /* from */, remaining_bytes)) {
      return -EFAULT;
   } else {
      /* Increase the position in the open file */
      *ppos += remaining_bytes;
      return remaining_bytes;
   }
}
 
static ssize_t
acme_write(struct file *file,const char __user *buf,size_t count,loff_t *ppos)
{
  int remaining_bytes;
 
   /* Number of bytes not written yet in the device */
   remaining_bytes = buf_size - (*ppos);
 
   if (count > remaining_bytes) {
      /* Can't write beyond the end of the device */
      return -EIO;
   }
 
   if (copy_from_user(*ppos+buffer /* to */, buf /* from */, count)) {
      return -EFAULT;
   } else {
      /* Increase the position in the open file */
      *ppos += count;
      return count;
   }
 
}
 
ssize_t aio_readtest (struct kiocb *f, const struct iovec *d, unsigned long r, loff_t l)
{
    printk("aio_read\n");
    return 0;
}
 
ssize_t aio_writetest(struct kiocb *f, const char __user *g, size_t h, loff_t j)
{
    printk("aio_write\n");
    return 0;
}
 
unsigned int example_poll(struct file * file, poll_table * pt)
{
    unsigned int mask = 0;
    if (data_avail_to_read) mask |= POLLIN | POLLRDNORM;
    if (data_avail_to_write) mask |= POLLOUT | POLLWRNORM;
    poll_wait(file, &hr, pt);
    poll_wait(file, &hw, pt);
    return mask;
}
static struct file_operations acme_fops =
{
    .owner = THIS_MODULE,
    .read = acme_read,
    .write = acme_write,
    .poll = example_poll,
    .aio_read = aio_readtest,
    .aio_write = aio_writetest,
};
 
static int acme_count = 1;
static dev_t acme_dev;
 
static struct cdev *acme_cdev;
 
static int
hello_init (void)
{
    buffer = kzalloc(8192,GFP_USER);
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
 
}
 
module_init (hello_init);
module_exit (hello_cleanup);
MODULE_LICENSE("GPL");

