//https://sysplay.in/blog/tag/wait-queues/
//http://derekmolloy.ie/writing-a-linux-kernel-module-part-2-a-character-device/
//http://lightsurge2.blogspot.in/2014/05/writing-linux-kernel-device-driver-for.html

#include <linux/init.h>         /* Macros used to mark up functions e.g. __init __exit */
#include <linux/module.h>       /*Core header for loading LKMs into the kernel*/
#include <linux/kernel.h>       /* printk() Contains types, macros, functions for the kernel*/
#include <linux/device.h>
#include <linux/wait.h>		/* wait_queue_head_t, wait_event_interruptible, wake_up_interruptible  */
#include <linux/sched.h>
#include <linux/delay.h>	/* usleep_range */
#include <linux/moduleparam.h>
#include <linux/device.h>       /* Header to support the kernel Driver Model*/
#include <linux/slab.h>         /* kmalloc() */
#include <linux/fs.h>           /* everything... Header for the Linux file system support*/
#include <linux/errno.h>        /* error codes */
#include <linux/types.h>        /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>        /* O_ACCMODE */
#include <linux/seq_file.h>
#include <linux/cdev.h>
#include <linux/string.h>	/* sprintf() and strlen() */
#include <asm/uaccess.h>        /* copy_from_user, copy_to_user */
#include "chardev_ioctl.h"	/* local definitions */
#include <uapi/linux/stat.h> 	/* S_IRUSR */
#include <linux/jiffies.h>
#include <linux/poll.h>
#include <linux/kthread.h>
#include <linux/mm.h>

static char readbuf[1024];
static size_t readbuflen;
static char writebuf[1024];
#if 0
static char flag = 'n';
#endif

static struct task_struct *kthread;
static wait_queue_head_t waitqueue;

#define  DEVICE_NAME "chardev" 			/* The device will appear at /dev/chardev using this value*/
#define CLASS_NAME  "chardev"   		/* The device class -- this is a character device driver */
#define CHARDEV_NR_DEVS   1
int chardev_major =   0;
int chardev_minor =   0;
struct chardev_dev *chardev_devices;        	/* allocated in init_module */
int chardev_nr_devs = CHARDEV_NR_DEVS;

static struct class*  charclass  = NULL; 	/* The device-driver class struct pointer */
static struct device* chardevice = NULL; 	/* The device-driver device struct pointer */

/* We define a mutex so that only one process at a time
* can access our driver at /dev. Any other process
* attempting to open this driver will return -EBUSY.
*/
static DEFINE_MUTEX(chardev_mutex);

# define  VM_RESERVED   (VM_DONTEXPAND | VM_DONTDUMP)
struct dentry  *file;
 
struct mmap_info {
	char *data;            
	int reference;      
};
 
void mmap_open(struct vm_area_struct *vma)
{
	struct mmap_info *info = (struct mmap_info *)vma->vm_private_data;
	info->reference++;
}
 
void mmap_close(struct vm_area_struct *vma)
{
	struct mmap_info *info = (struct mmap_info *)vma->vm_private_data;
	info->reference--;
}
 
static int mmap_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{
	struct page *page;
	struct mmap_info *info;    
     
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

struct vm_operations_struct mmap_vm_ops = {
	.open =     mmap_open,
	.close =    mmap_close,
	.fault =    mmap_fault,
};

int chardev_mmap(struct file *filp, struct vm_area_struct *vma)
{
	vma->vm_ops = &mmap_vm_ops;
	vma->vm_flags |= VM_RESERVED;    
	vma->vm_private_data = filp->private_data;
	mmap_open(vma);
	return 0;
}

/* try to reverse a string without using swap or temp variable ? */
int string_rev(char *str)
{
        int len, i, j, swap = 0;

        printk(KERN_ALERT "Chardev: %s : caller: %pS\n",__func__, __builtin_return_address(0));
        len = strlen(str);
        for (i = 0, j = (len-1); i < (len/2); i++, j--) {
                swap = str[i];
                str[i] = str[j];
                str[j] = swap;
        }
        return 0;
}

/* called when 'ioctl' system call is done on device file */
static long chardev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
        char *tmp_ptr = NULL;
        static int buf_size = 0;
        static char *kern_buf = NULL;
        int rv;
        struct string tmp_s;

        printk(KERN_ALERT "Chardev: %s : caller: %pS\n",__func__, __builtin_return_address(0));

        switch (cmd) {
                case CD_IOC_ALLOC_BUF:	/*define all ioctl command in chardev_ioctl.h*/
                        buf_size = arg;
                        kern_buf = (char *) kmalloc(buf_size, GFP_USER);
                        printk(KERN_ALERT "Chardev: buf_size %d kern_buf %p\n", buf_size, kern_buf);
                        break;

                case CD_IOC_WRITE_STRING:
                        tmp_ptr = (char *)arg;
                        copy_from_user(kern_buf, tmp_ptr, buf_size);
                        printk(KERN_ALERT "Chardev: CD_IOC_WRITE_STRING: kern_buf %s\n", kern_buf);
                        break;

                case CD_IOC_REVERSE_STRING:
                        rv = string_rev(kern_buf);
                        printk(KERN_ALERT "Chardev: CD_IOC_REVERSE_STRING: kern_buf %s\n", kern_buf);
                        break;

                case CD_IOC_READ_STRING:
                        copy_to_user((char *) arg, kern_buf, buf_size);
                        break;
                        case CD_IOC_FREE_BUF:
                        kfree(file);
                        break;

                case CD_IOC_READ_WRITE_REVERSE_STRING:
                        copy_from_user(&tmp_s, (struct string *) arg, sizeof(tmp_s));
                        printk(KERN_ALERT "Chardev: String from user space %s\n", tmp_s.original);
                        rv = string_rev(tmp_s.original);
                        printk(KERN_ALERT "Chardev: Reversed String %s\n", tmp_s.original);
                        copy_to_user(&(((struct string *) arg)->reverse), tmp_s.original, sizeof(tmp_s.original));
                        break;

                default:
                        return -1;
        }
        return 0;
}

/*
If you return 0 here, then the kernel will sleep until an event happens in the queue.
This gets called again every time an event happens in the wait queue.
*/
unsigned int chardev_poll (struct file *file, struct poll_table_struct *wait)
{
        printk(KERN_ALERT "Chardev: %s : caller: %pS\n",__func__, __builtin_return_address(0));
	poll_wait(file, &waitqueue, wait);
	if (readbuflen)
		return POLLIN;
	else
		return 0;
}

static int kthread_func(void *data)
{
        printk(KERN_ALERT "Chardev: %s : caller: %pS\n",__func__, __builtin_return_address(0));
	while (!kthread_should_stop()) {
		readbuflen = snprintf(readbuf, sizeof(readbuf), "%llu", (unsigned long long)jiffies);
		usleep_range(1000000, 1000001);
		wake_up(&waitqueue);
	}
	return 0;
}

/** @brief The device open function that is called each time the device is opened
 *  This will only increment the numberOpens counter in this case.
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
int chardev_open(struct inode *inode, struct file *file)
{
        struct chardev_dev *dev; /* device information */

        printk(KERN_ALERT "Chardev: %s : caller: %pS\n",__func__, __builtin_return_address(0));

	/* Our driver only allows writing */
	if ((file->f_flags & O_ACCMODE) != O_WRONLY)
		return -EACCES;

	/* We need to ensure that only one process can access the file handle at one time */
	if (!mutex_trylock(&chardev_mutex)) {
		printk("%s: Device currently in use!\n", __func__);
		return -EBUSY;
	}

        dev = container_of(inode->i_cdev, struct chardev_dev, cdev);
        file->private_data = dev; /* for other methods */

        dev->count++;
        printk(KERN_INFO "Chardev: Device has been opened %d time(s)\n", dev->count);

	/* mmap spacific */
	struct mmap_info *info = kmalloc(sizeof(struct mmap_info), GFP_KERNEL);    
	info->data = (char *)get_zeroed_page(GFP_KERNEL);
	memcpy(info->data, "hello from kernel this is file: ", 32);
	memcpy(info->data + 32, file->f_path.dentry->d_name.name, strlen(file->f_path.dentry->d_name.name));
	/* assign this info struct to the file */
	file->private_data = info;

	return 0;
}

/** @brief The device release function that is called whenever the device is closed/released by
 *  the userspace program
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
int chardev_release(struct inode *inode, struct file *filp) 
{
	printk("%s: Freeing /dev resource\n", __func__);
	
	/* mmap spacific */	
	struct mmap_info *info = filp->private_data;
     	free_page((unsigned long)info->data);
	kfree(info);
	filp->private_data = NULL;

	mutex_unlock(&chardev_mutex);
	return 0;
}

/** @brief This function is called whenever device is being read from user space i.e. data is
 *  being sent from the device to the user. In this case is uses the copy_to_user() function to
 *  send the buffer string to the user and captures any errors.
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 *  @param buffer The pointer to the buffer to which this function writes the data
 *  @param len The length of the b
 *  @param offset The offset if required
 */
ssize_t chardev_read(struct file *filp, char *buff, size_t count, loff_t *offp) 
{
	ssize_t ret;
	if (copy_to_user(buff, readbuf, readbuflen)) {
		ret = -EFAULT;
	} else {
		ret = readbuflen;
	}
	/* This is normal pipe behaviour: data gets drained once a reader reads from it. */
	/* https://stackoverflow.com/questions/1634580/named-pipes-fifos-on-unix-with-multiple-readers */
	readbuflen = 0;
	return ret;

	#if 0
	printk(KERN_INFO "Inside read\n");
	printk(KERN_INFO "Scheduling Out\n");
	wait_event_interruptible(waitqueue, flag == 'y');
	flag = 'n';
	printk(KERN_INFO "Woken Up\n");
	return 0;
	#endif
}

/** @brief This function is called whenever the device is being written to from user space i.e.
 *  data is sent to the device from the user. The data is copied to the message[] array in this
 *  LKM using the sprintf() function along with the length of the string.
 *  @param filep A pointer to a file object
 *  @param buffer The buffer to that contains the string to write to the device
 *  @param len The length of the array of data that is being passed in the const char buffer
 *  @param offset The offset if required
 */
ssize_t chardev_write(struct file *filp, const char *buff, size_t count, loff_t *offp) 
{   
	printk(KERN_INFO "Inside write\n");
	if (copy_from_user(writebuf, buff, sizeof(count)))
	{
		return -EFAULT;
	}
	printk(KERN_INFO "%s", writebuf);
	
	#if 0
	printk(KERN_INFO "%c", flag);
	wake_up_interruptible(&waitqueue);
	#endif
	return strlen(writebuf);
}

/** @brief Devices are represented as file structure in the kernel. The file_operations structure from
 *  /linux/fs.h lists the callback functions that you wish to associated with your file operations
 *  using a C99 syntax structure. char devices usually implement open, read, write and release calls
 */
/*What is the difference between ioctl(), unlocked_ioctl() and compat_ioctl()?*/
struct file_operations chardev_fops = {
	.read 		= chardev_read,
	.write 		= chardev_write,
	.open 		= chardev_open,
	.release	= chardev_release,
	.poll 		= chardev_poll,
	.unlocked_ioctl = chardev_ioctl,
	.mmap 		= chardev_mmap,
};

/* Set up the char_dev structure for this device */
static void chardev_setup_cdev(struct chardev_dev *dev, int index)
{
        int err = 0; 
	dev_t devno = MKDEV(chardev_major, chardev_minor + index);

        printk(KERN_ALERT "Chardev: %s : caller: %pS\n",__func__, __builtin_return_address(0));

        cdev_init(&dev->cdev, &chardev_fops);
        dev->cdev.owner = THIS_MODULE;
        dev->cdev.ops = &chardev_fops;
        err = cdev_add(&dev->cdev, devno, 1);
        /* Fail gracefully if need be. */
        if (err)
                printk(KERN_NOTICE "Chardev: Error %d adding scull%d", err, index);
}

/* Our driver attributes/variables are currently exported via sysfs. 
 * For this driver, we export two attributes - chip_show and char_store
 * The sysfs filesystem is a convenient way to examine these attributes
 * in kernel space from user space. They also provide a mechanism for 
 * setting data form user space to kernel space. 
 **/
static ssize_t chardev_store(struct device *dev, struct device_attribute * devattr, const char * buf, size_t count)
{
	int err = 0;
	int value = 0;
        printk(KERN_ALERT "Chardev: %s : caller: %pS\n",__func__, __builtin_return_address(0));

	/* kstrtol, sprintf, strlen */
	err = kstrtoint(buf, 10, &value);
	if (err < 0)
		return err;

	return err;
}

static ssize_t chardev_show(struct device *dev, struct device_attribute *dev_attr, char * buf)
{
	int value = 0;
        printk(KERN_ALERT "Chardev: %s : caller: %pS\n",__func__, __builtin_return_address(0));

	/* Copy the result back to buf */
	return sprintf(buf, "%d\n", value);
}

/*Create device attribute for function show and store */
static DEVICE_ATTR(chardev_file, S_IRUGO /* read only*/ | S_IWUSR /* write only */, chardev_show, chardev_store);

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void __exit chardev_cleanup(void)
{
        dev_t devno = MKDEV(chardev_major, chardev_minor);
        printk(KERN_ALERT "Chardev: %s : caller: %pS\n",__func__, __builtin_return_address(0));

	device_remove_file(chardevice, &dev_attr_chardev_file);
	printk("Chardev: device_remove_file succesfully\n");

	kthread_stop(kthread);
	printk("Chardev: kthread_stop succesfully\n");
	
        if (charclass) {
                /* destroy the device */
                device_destroy(charclass, devno);
                /* destroy the device class */
                class_destroy(charclass);
                printk("Chardev: class and device freed succesfully\n");
        }

        /* Get rid of our char dev entries. */
        if (chardev_devices) {
                cdev_del(&chardev_devices->cdev);
                kfree(chardev_devices);
                printk("Chardev: cdev resource freed succesfully\n");
        }

        /* cleanup_module is never called if registering failed. */
        unregister_chrdev_region(devno, chardev_nr_devs);
        printk("Chardev: cihardev unregisterd succesfully\n");

}

/** @brief The LKM initialization function
 *  The static keyword restricts the visibility of the function to within this C file. The __init
 *  macro means that for a built-in driver (not a LKM) the function is only used at initialization
 *  time and that it can be discarded and its memory freed up after that point.
 *  @return returns 0 if successful
 */
static int __init chardev_init (void)
{
	int ret;
	static dev_t dev = 0;

	/* We now create our character device driver */
	if ((ret = alloc_chrdev_region(&dev, chardev_minor, chardev_nr_devs, "chardev")) < 0)
	{
		return ret;
	}
	printk("Chardev: alloc_chrdev_region: Major Nr: %d\n", MAJOR(dev));
	chardev_major = MAJOR(dev);

        /* 
         * Allocate the devices. This must be dynamic as the device number can
         * be specified at load time.
         */
        chardev_devices = kmalloc(chardev_nr_devs * sizeof(struct chardev_dev), GFP_KERNEL);
        if (!chardev_devices) {
                printk(KERN_WARNING "Chardev: chardev kamlloc failed\n");
                ret = -ENOMEM;
                goto fail;
        }
        memset(chardev_devices, 0, chardev_nr_devs * sizeof(struct chardev_dev));

	/* Initialize the mutex for /dev fops clients */
	mutex_init(&chardev_mutex);

        /* Initialize each device */
        chardev_setup_cdev(chardev_devices, 0);

	/* The PTR_ERR() is a function that is defined in linux/err.h that retrieves the error number from the pointer. */
        charclass = class_create(THIS_MODULE, CLASS_NAME /*chardev*/);
        if (IS_ERR(charclass)){
                printk(KERN_ALERT "Chardev: Failed to register device class\n");
                /* Correct way to return an error on a pointer */
                ret = PTR_ERR(charclass);
                goto fail;
        }
        printk(KERN_INFO "Chardev: device class registered correctly\n");

        chardevice = device_create(charclass, NULL, MKDEV(chardev_major, 0), NULL, DEVICE_NAME);
        if (IS_ERR(chardevice)) {
                printk(KERN_ALERT "Chardev: Failed to create the device\n");
                ret = PTR_ERR(chardevice);
                goto fail;
        }
        printk(KERN_INFO "Chardev: device class created correctly\n");

	init_waitqueue_head(&waitqueue);
	kthread = kthread_create(kthread_func, NULL, "mykthread");
	wake_up_process(kthread);
	printk("kernel thread mykthread created successfully");

	/* Initialize the mutex for /dev fops clients */
	
	/* register our sysfs attributs */
	ret = device_create_file(chardevice, &dev_attr_chardev_file);
        if (ret) {
		pr_err("chardev: device attr create error: %d.\n", ret);
		goto fail;
        }

        return 0; /* succeed */

/* Cleanup on failed operations */
fail:
        chardev_cleanup();
        return ret;
	
	return 0;
}

/** @brief A module must use the module_init() module_exit() macros from linux/init.h, which
 *  identify the initialization function at insertion time and the cleanup function (as
 *  listed above)
 */
module_init(chardev_init);
module_exit(chardev_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dharmender Sharma");
MODULE_DESCRIPTION("character test device");
