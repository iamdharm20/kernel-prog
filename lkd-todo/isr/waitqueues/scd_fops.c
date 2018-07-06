/* https://stackoverflow.com/questions/36358152/bus-error-when-reading-register-using-mmap
$ insmod wait.ko

wait in read untill write is not done 
$ cat /dev/mychar0

This gets our process blocked. Open another shell to wake up the process:
$ sudo -i
$ echo 'y' > /dev/mychar0
*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/debugfs.h>
#include <linux/slab.h>
#include <asm/uaccess.h> /* copy_from_user, copy_to_user */
#include <linux/delay.h> /* usleep_range */
#include <linux/errno.h> /* EFAULT */
#include <linux/jiffies.h>
#include <linux/kthread.h>
#include <linux/poll.h>
#include <linux/printk.h> /* printk */
#include <linux/wait.h> /* wait_queue_head_t, wait_event_interruptible, wake_up_interruptible  */


#define FIRST_MINOR 0
#define MINOR_CNT 1
#define MODULE_NAME             "SCD"
# define  VM_RESERVED   (VM_DONTEXPAND | VM_DONTDUMP)

/* --------------------------------------------------------------
 *              External References
 * ------------------------------------------------------------*/

/* --------------------------------------------------------------
 *              Application Includes
 * ------------------------------------------------------------*/

/* --------------------------------------------------------------
 *              Constants Definition
 * ------------------------------------------------------------*/

/* --------------------------------------------------------------
 *              Macros Definition
 * ------------------------------------------------------------*/

/* --------------------------------------------------------------
 *              Static data
 * ------------------------------------------------------------*/

/* --------------------------------------------------------------
 *              Local Functions Definition
 * ------------------------------------------------------------*/

/* --------------------------------------------------------------
 *              spacify internal or private  data structure
 * ------------------------------------------------------------*/

/* --------------------------------------------------------------
 * 		Kernel module information
 * ------------------------------------------------------------*/

static char flag = 'n';
static dev_t dev;
static struct cdev c_dev;
static struct class *cl;
static DECLARE_WAIT_QUEUE_HEAD(wq);
struct dentry  *file;
static char readbuf[1024];
static size_t readbuflen;
static struct task_struct *kthread;
struct device *dev_ret;

/* spacify internal or private  data structures */
struct mmap_info
{
	char *data;            
	int reference;      
};
 
/* A mutex will ensure that only one process accesses our device */
static DEFINE_MUTEX(mmap_device_mutex);

/*****************************************************************************
 * Open function of the module which makes this module accessible from the
 * user space.
 *
 * @return
 *      SUCCESS : 0
 *      FAILURE : Negative error code.
 *****************************************************************************/
static int open(struct inode *inode, struct file *filp)
{
	struct mmap_info *info = kmalloc(sizeof(struct mmap_info), GFP_KERNEL);    
	printk(KERN_INFO "SCD: %s\n", __func__);

	if (!mutex_trylock(&mmap_device_mutex)) {
		printk(KERN_WARNING
		       "Another process is accessing the device\n");
		return -EBUSY;
	}	

	info->data = (char *)get_zeroed_page(GFP_KERNEL);
	memcpy(info->data, "hello from kernel this is file: ", 32);
	memcpy(info->data + 32, filp->f_path.dentry->d_iname, strlen(filp->f_path.dentry->d_iname));
	/* assign this info struct to the file */
	filp->private_data = info;
	return 0;
}

/*****************************************************************************
 * Close function of the module which releases the use of this module from
 * the user space.
 *
 * @return
 *      SUCCESS : 0
 *      FAILURE : Negative error code.
 *****************************************************************************/
static int release(struct inode *inode, struct file *filp) 
{
	struct mmap_info *info = filp->private_data;
	printk(KERN_INFO "SCD: %s\n", __func__);
     
	free_page((unsigned long)info->data);
	kfree(info);
	filp->private_data = NULL;

	mutex_unlock(&mmap_device_mutex);

	return 0;
}

static ssize_t read(struct file *filp, char *buff, size_t count, loff_t *offp) 
{
	int ret = 0;

	printk(KERN_INFO "SCD: %s\n", __func__);

        if (copy_to_user(buff, readbuf, readbuflen)) {
                ret = -EFAULT;
        } else {
                 ret = readbuflen;
        }
        readbuflen = 0;

	printk(KERN_INFO "Scheduling Out\n");
	wait_event_interruptible(wq, flag == 'y');
	flag = 'n';
	printk(KERN_INFO "Woken Up\n");
	
	return 0;
}

static ssize_t write(struct file *filp, const char *buff, size_t count, loff_t *offp) 
{   
	printk(KERN_INFO "SCD: %s\n", __func__);
	if (copy_from_user(&flag, buff, 1))
	{
		return -EFAULT;
	}
	printk(KERN_INFO "%c", flag);
	wake_up_interruptible(&wq);
	return count;
}
/*
If you return 0 here, then the kernel will sleep until an event happens in the queue.
This gets called again every time an event happens in the wait queue.
read poll.c for more information.
*/
unsigned int poll(struct file *filp, struct poll_table_struct *wait)
{
        /*TODO*/
        /*wait_event_interruptible(waitqueue, (dev->rp != dev->wp));*/
        pr_info("poll_wait before\n");
        poll_wait(filp, &wq, wait);
        pr_info("poll_wait after\n");
        /* return POLLIN | POLLRDNORM if you have some new data to read, and 0 in case there is no new data to read */
        if (readbuflen)
                return POLLIN | POLLRDNORM;
        else
                return 0;

}

/* this tread is used for poll func */
static int kthread_func(void *data)
{
        while (!kthread_should_stop()) {
                readbuflen = snprintf(readbuf, sizeof(readbuf), "%llu", (unsigned long long)jiffies);
                usleep_range(1000000, 1000001);
                /* Notify your waitqueue once you have new data */
        //      wake_up_interruptible(&waitqueue);
                wake_up(&wq);
        }
        return 0;
}


static void my_mmap_open (struct vm_area_struct* vma) 
{
	struct mmap_info *info = (struct mmap_info *)vma->vm_private_data;
	
	printk(KERN_INFO "SCD: %s\n", __func__);
	printk("%s VMA open virtAddr=%#lx physAddr=%#lx", MODULE_NAME, vma->vm_start, vma->vm_pgoff<<PAGE_SHIFT);
	info->reference++;

}

static void my_mmap_close (struct vm_area_struct* vma) 
{
	struct mmap_info *info = (struct mmap_info *)vma->vm_private_data;
	
	printk(KERN_INFO "SCD: %s\n", __func__);
	info->reference--;

	#if 0 /* commented as all clenup is done in release function*/	
	struct mmap_info *info = filp->private_data;
	/* obtain new memory */
	free_page((unsigned long)info->data);
    	kfree(info);
	filp->private_data = NULL
	#endif
}

static int my_mmap_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{
	struct page *page;
	struct mmap_info *info;    
     
	printk(KERN_INFO "SCD: %s\n", __func__);
	info = (struct mmap_info *)vma->vm_private_data;
	if (!info->data) {
		printk("No data\n");
		return 0;    
	}
     
	page = virt_to_page(info->data);    
     
	get_page(page);
	vmf->page = page; 

	return 0;
}     

/*
 * Specifies the functions associated with the remap operations.
 */
static struct vm_operations_struct mmap_vm_ops = {
        .open = my_mmap_open,
        .close = my_mmap_close,
	.fault = my_mmap_fault,
};

/*****************************************************************************
 * Creates a new mapping in the virtual address space of the calling process.
 * It makes possible for a user process to access physical memory in the
 * kernel space.
 *
 * @param   filp
 *              The file or device.
 * @param   vma
 *              The virtual memory area into which the page range is being
 *              mapped.
 * @return
 *      SUCCESS : 0
 *      FAILURE : Negative error code.
 *****************************************************************************/
static int mmap(struct file *filp, struct vm_area_struct *vma)
{
	printk(KERN_INFO "SCD: %s\n", __func__);
	#if 0
	/*mmap_app.c spacific changes */
	/*
	vma: user vma to map to 
	addr: target user address to start at 
	pfn: physical address of kernel memory 
	size: size of map area 
	prot: page protection flags for this mapping 
	*/
	if(remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,
	                    vma->vm_end - vma->vm_start, vma->vm_page_prot) ) {
                return -EAGAIN;
	}
	#endif

	/* spacify mmap file operation */	
	vma->vm_ops = &mmap_vm_ops;
	vma->vm_flags |= VM_RESERVED;

	/* assign the file private data to the vm private data */ 
	vma->vm_private_data = filp->private_data;

	/* call explicitely my_mmap_open as its not done by calling mmap() */
	my_mmap_open(vma);

	return 0;
}

/*
 * Specifies the functions associated with the device operations.
 */
struct file_operations pra_fops = {
	.owner 		= THIS_MODULE,
	.read 		= read,
	.write 		= write,
	.open 		= open,
	.release	= release,
	.mmap 		= mmap,
	.poll 		= poll,
};

ssize_t scd_show(struct device *dev, struct device_attribute *attr, char *buffer)
{
	printk(KERN_INFO "SCD: %s\n", __func__);
        return 0;
}

ssize_t scd_store(struct device *dev, struct device_attribute *attr, const char *buffer, size_t len)
{
	printk(KERN_INFO "SCD: %s\n", __func__);
        return 0;
}

static DEVICE_ATTR(scd_testfile, S_IRUGO | S_IWUSR, scd_show, scd_store);

/*****************************************************************************
 * Initialization function of the module which allocates the major and minor
 * numbers and registers the device to /proc/devices. The creation of the
 * device in /dev must be done by an external script.
 *
 * @return
 *      SUCCESS : 0
 *      FAILURE : Negative error code.
 *****************************************************************************/
static int __init scd_init (void)
{
	int ret;

	printk(KERN_INFO "SCD: %s\n", __func__);

	/* Registers a range of device numbers.
        ret = register_chrdev_region(dev, NB_DEVICES, MODULE_NAME)
	*/

	/* If major number is 0, then allocates it dynamically for this device 
	* you can allocate a range of character device number chosen dynamically
	*/
	if ((ret = alloc_chrdev_region(&dev, FIRST_MINOR, MINOR_CNT/* number of devices*/, MODULE_NAME)) < 0)
	{
		return ret;
	}
	printk("Registered device Major Nr: %d\n", MAJOR(dev));

	/* Initializes cdev and file operation */	
	cdev_init(&c_dev, &pra_fops);

	/** Registers the device, makes it live immediately, therefore all
	 * initialization routines must be done before calling this function
	 * so Adds the device to the system making it live immediately
	*/
	if ((ret = cdev_add(&c_dev, dev, MINOR_CNT)) < 0)
	{
		
	        printk("Could not allocate chrdev");
		unregister_chrdev_region(dev, MINOR_CNT);
		return ret;
	}
	printk("Allocated chrdev for %s", MODULE_NAME);

	if (IS_ERR(cl = class_create(THIS_MODULE, "scd")))
	{
		cdev_del(&c_dev);
		unregister_chrdev_region(dev, MINOR_CNT);
		return PTR_ERR(cl);
	}
	printk("/sys/class/scd created\n");

	if (IS_ERR(dev_ret = device_create(cl, NULL, dev, NULL, "chardev%d", 0)))
	{
		class_destroy(cl);
		cdev_del(&c_dev);
		unregister_chrdev_region(dev, MINOR_CNT);
		return PTR_ERR(dev_ret);
	}
	printk("/dev/chardev0 created\n");

        kthread = kthread_create(kthread_func, NULL, "scdkthread");
        wake_up_process(kthread);
        if (kthread)
                printk("scdkthread Created successfully:ps -elf | grep scd\n");
        else
                printk("Thread creation failed\n");

        ret = device_create_file(dev_ret, &dev_attr_scd_testfile);
        if (ret) {
                pr_err("scd device attr create error: %d.\n", ret);
        }
	printk("/sys/class/scd/chardev0/scd_testfile created\n");


	mutex_init(&mmap_device_mutex);
	
	return 0;
}


/*****************************************************************************
 * Cleanup function of the module which unregisters the major number and
 * removes the created device from the system.
 *
 * @return
 *      void
 *****************************************************************************/
static void __exit scd_exit(void)
{
	printk(KERN_INFO "SCD: %s\n", __func__);

	device_remove_file(dev_ret, &dev_attr_scd_testfile);
	printk("/sys/class/chardev/scd_testfile removed\n");

	/* Removed charter Node */
	device_destroy(cl, dev);

	class_destroy(cl);
	
	printk("Removed the cdev from the system");	
	cdev_del(&c_dev);
	
	printk("Unregistered device %s", MODULE_NAME);	
	unregister_chrdev_region(dev, MINOR_CNT);

        if (kthread != NULL)
        {
                kthread_stop(kthread);
                printk(KERN_INFO "Thread stopped");
        }

}

module_init(scd_init);
module_exit(scd_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Waiting Process Demo");
