#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/gfp.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/kdev_t.h>
#include <linux/delay.h>
#include <linux/ioctl.h>
#include <linux/slab.h>
#include <linux/mempool.h>
#include <linux/mm.h>
#include <asm/io.h>
 
 
static dev_t my_dev;
static struct cdev *my_cdev;
 
 
// callback for read system call on the device
static ssize_t my_read(struct file *file, char __user *buf,size_t count,loff_t *ppos)
{
   int len=5;
   if(*ppos > 0)
   {
	return 0;
   }
   mdelay(5000); // busy-wait for 5 seconds
//   might_resched();
//   delay(3000);  // busy wait for 3 seconds 

   if (copy_to_user(buf , "hello" , len)) {
      return -EFAULT;
   } else {
       *ppos +=len;
       return len;
   }
}
 
 
 
static struct file_operations my_fops =
{
	.owner = THIS_MODULE,
	.read = my_read,
};
 
 
 
 
static int __init hello_init (void)
{
 
	my_dev = MKDEV(400,0);
	register_chrdev_region(my_dev,1,"demo");
 
	my_cdev=cdev_alloc();
	if(!my_cdev)
	{
		printk (KERN_INFO "cdev alloc error.\n");
 		return -1;		
	}
	my_cdev->ops = &my_fops;
	my_cdev->owner = THIS_MODULE;
	
	if(cdev_add(my_cdev,my_dev,1))
	{
		printk (KERN_INFO "cdev add error.\n");
 		return -1;		
	}
	
 
  return 0;
 
}
 
 
static void __exit hello_cleanup (void)
{
	cdev_del(my_cdev);
	unregister_chrdev_region(my_dev, 1);
}
 
 
module_init(hello_init);
module_exit(hello_cleanup);
MODULE_LICENSE("GPL");
