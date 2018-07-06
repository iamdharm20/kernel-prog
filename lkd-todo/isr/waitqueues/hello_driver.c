/*
 * Driver for the hello device. This driver is intended to show
 * and teach some basic kernel mechanics :
 * _ Linux Kernel Module programming
 * _ Driver programming
 * _ Debugfs operations
 * _ Semaphores Locking
 * _ Wait queues
 *
 * You now have to create the device node :
 *    mknod /dev/hello c X 0
 *
 * How to use the hello device ?
 * Once you've loaded the module, there is "Nothing" in the device. If you try to read
 * it, the process will block.
 * It will block until another process write some data in the device.
 * You can see the device's data by reading the device or by the debugfs file system.
 * You can write to the data by writing to the device or by the debugfs interface.
 * To reset the device, write "Nothing\n" in the device.
 *
 * I provide two userland programm to read and write to the hello device :
 * _ read_to_hello (which can only read 256 bytes)
 * _ write_to_hello
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/errno.h>
#include <linux/debugfs.h>

#define DRIVER_DESC "Hello World Driver"
#define DEVICE_NAME "hello"

#define NOTH_LEN 9

static int major;
static int count_user;
static char *hello_msg; 
static struct semaphore hello_sem;
static wait_queue_head_t attente;
static wait_queue_head_t inq;
static struct dentry *hello_dir;
static struct dentry *hello_dentry;

static int hello_open(struct inode *inode, struct file *file)
{
        down(&hello_sem);
        count_user++;
        up(&hello_sem);

        return 0;
}       

static int hello_release(struct inode *inode, struct file *file)
{
        down(&hello_sem);
        count_user--;
        up(&hello_sem);

        return 0;
}       

static ssize_t hello_read(struct file *filp, char __user *user_buf, size_t len, loff_t *offset)
{
        int bytes = 0;

        down(&hello_sem);
        while (strncmp(hello_msg, "Nothing\n", NOTH_LEN) == 0) {
                up(&hello_sem);
                printk(KERN_INFO "Process sleeps.\n");
                interruptible_sleep_on(&attente);
                printk(KERN_INFO "Process wakes up.\n");
                down(&hello_sem);
        }       
        bytes = copy_to_user(user_buf, hello_msg, len);
        if (bytes < 0) {
                up(&hello_sem);
                return -EFAULT;
        }
        up(&hello_sem);

        return bytes;
}       

static ssize_t hello_write(struct file *filp, const char __user *user_buf, size_t len, loff_t *offset)
{
        int bytes = 0;

        down(&hello_sem);
        if (hello_msg) {
                kfree(hello_msg);
                hello_msg = kmalloc(len+1, GFP_KERNEL);
                if (!hello_msg) {
                        up(&hello_sem);
                        return -ENOMEM;
                }
        }        
        
      bytes = copy_from_user(hello_msg, user_buf, len);
        if (bytes < 0) {
                up(&hello_sem);
                return -EFAULT;
        }
        hello_msg[len] = '\0';
        up(&hello_sem);
        wake_up_interruptible(&attente);

        return bytes;
}

static unsigned int hello_poll(struct file *filp, poll_table *wait) 
{
        unsigned int mask = 0;

        poll_wait(filp, &inq, wait);
        if (strncmp(hello_msg, "Nothing\n", NOTH_LEN) != 0)
                mask |= POLLIN | POLLRDNORM;
        mask |= POLLOUT | POLLWRNORM;

        return mask;
}       

static loff_t hello_llseek(struct file *filp, loff_t off, int origin)  
{
        switch (origin) {
                default:
                        off += filp->f_pos;
                        break;
        }

        if (off < 0) {
                return -EINVAL;
        }
        filp->f_pos = off;

        return filp->f_pos;
}       

static int debug_hello_open(struct inode *inode, struct file *file)
{
        return 0;
}

static int debug_hello_release(struct inode *inode, struct file *file)
{
        return 0;
}

static ssize_t debug_hello_read(struct file *file, char __user *user_buf, size_t count, loff_t *off)
{
        int len;

        down(&hello_sem);
        len = copy_to_user(user_buf,hello_msg,count);
        up(&hello_sem);

        if (len < 0) {
                return -EFAULT;
        }

        return len;
}

static struct file_operations fops = {
        read:   hello_read,
        write:  hello_write,
        open:   hello_open,
        release: hello_release,
        poll:   hello_poll,     
        llseek: hello_llseek,
};               

static struct file_operations debug_fops = {
        read:   debug_hello_read,
        open:   debug_hello_open,
        release: debug_hello_release,
};

int __init loading(void)
{      
        hello_msg = kmalloc(NOTH_LEN, GFP_KERNEL);
        if (!hello_msg) {
                return -ENOMEM;
        }
       
        sprintf(hello_msg, "Nothing\n");
        
        major = register_chrdev(major, DEVICE_NAME, &fops);
        if (major < 0) {
                printk(KERN_ERR "Cannot allocate a major number.\n");
                return major;
        }       
        printk(KERN_INFO "Major number %d has been assigned to device %s.\n", major, DEVICE_NAME);

        sema_init(&hello_sem, 1);
        init_waitqueue_head(&attente);
        init_waitqueue_head(&inq);

        hello_dir = debugfs_create_dir("hello_dir", NULL);
        hello_dentry = debugfs_create_file("hello", 0644, hello_dir, NULL, &debug_fops);

        return 0; 
}

void __exit unloading(void)
{
        int ret = 0;
        ret = unregister_chrdev(major, DEVICE_NAME);

        if (ret < 0) {
                printk(KERN_ERR "unregister_chrdev() error.\n");
        }       

        debugfs_remove(hello_dentry);
        debugfs_remove(hello_dir);

        kfree(hello_msg);
}       

module_init(loading);
module_exit(unloading);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION(DRIVER_DESC);

