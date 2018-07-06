#include <linux/init.h>		/* Macros used to mark up functions e.g. __init __exit */
#include <linux/module.h>	/* Core header for loading LKMs into the kernel */
#include <linux/kernel.h>	/* Contains types, macros, functions for the kernel */
#include <linux/fs.h>		/*  Header for the Linux file system support */
#include <linux/uaccess.h>	/* Required for the copy to user function */
#include <linux/interrupt.h>	
#include <linux/version.h>
#include <linux/reboot.h>
#include <linux/slab.h>
#include <linux/device.h>	/* Header to support the kernel Driver Model */
#include <linux/delay.h>        /* for msleep() */
#include <linux/timer.h>        /* for timers apis */
#include <asm/siginfo.h>        //siginfo
#include <linux/sched.h>        //find_task_by_pid_type
#include <linux/kthread.h>

#include "ledchar_ioctl.h"

#define  DEVICE_NAME "ledchar"    /* The device will appear at /dev/ledchar using this value */
#define  CLASS_NAME  "ledc"        /* The device class -- this is a character device driver */

#define SIG_TEST 44     // we choose 44 as our signal number (real-time signals are in the range of 33 to 64)

static int    majorNumber;                  /* Stores the device number -- determined automatically */
static char   message[256] = {0};           /* Memory for the string that is passed from userspace */
static short  size_of_message;              /* Used to remember the size of the string stored */
static int    numberopens = 0;              /* Counts the number of times the device is opened */
static struct class*  ledcharClass  = NULL; 
static struct device* ledcharDevice = NULL;
static DEFINE_MUTEX(ledchar_mutex);  

static struct task_struct *thread_st;

static int signal_to_pid(int pid)
{
        int ret;
        struct siginfo info;
        struct pid *pid_struct;
        struct task_struct *task;

	pr_info("Chardev: pid = %d\n", pid);
	
	pr_info("Chardev: %s\n", __func__);

        /* send the signal */
        memset(&info, 0, sizeof(struct siginfo));
        info.si_signo = SIG_TEST;
        info.si_code = SI_QUEUE;

	info.si_int = 1234;

        pid_struct = find_get_pid(pid);

        /* find the task_struct associated with this pid */
        task = pid_task(pid_struct, PIDTYPE_PID);
        if(task == NULL){
                pr_info("Chardev: no such pid\n");
		return -ENODEV;
	}
	
	/* send the signal */
	send_sig_info(SIG_TEST, &info, task);
        if (ret < 0) {
                pr_info("Chardev: error sending signal\n");
                return ret;
        }

	return 0;
}

static int thread_fn(void *data)
{
        /* Allow the SIGKILL signal kill -9 <thread_id>, ps -elf | grep thread */
        allow_signal(SIGKILL);

        while (!kthread_should_stop()) {
        	pr_info("Chardev: %s\n",__func__);
		ssleep(5);
                if (signal_pending(current)) {
			pr_info("Chardev: signal_pending got\n");
			break;
		}
                //usleep_range(1000000, 1000001);
                //wake_up(&waitqueue);
        }
	pr_info("Chardev: Thread Stopping\n");
	thread_st = NULL;
        do_exit(0);
}

/* try to reverse a string without using swap or temp variable ? */
static int string_rev(char *str)
{
        int len, i, j, swap = 0;

	pr_info("Chardev: %s\n", __func__);
        len = strlen(str);
        for (i = 0, j = (len-1); i < (len/2); i++, j--) {
                swap = str[i];
                str[i] = str[j];
                str[j] = swap;
        }
        return 0;
}

static long dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
        char *tmp_ptr = NULL;
        static int buf_size = 0;
        static char *kern_buf = NULL;
        int rv;
	int pid = 0;

        struct string tmp_s;

	pr_info("Chardev: %s\n", __func__);

	switch (cmd) {
		case CD_SIGNAL_GEN:
			pid = arg;
			signal_to_pid(pid);
			break;
		
		case CD_IOC_ALLOC_BUF:  /*define all ioctl command in led_ioctl.h*/
                        buf_size = arg;
                        kern_buf = (char *) kmalloc(buf_size, GFP_USER);
                        pr_alert("Chardev: buf_size %d kern_buf %p\n", buf_size, kern_buf);
                        break;

                case CD_IOC_WRITE_STRING:
                        tmp_ptr = (char *)arg;
                        copy_from_user(kern_buf, tmp_ptr, buf_size);
                        pr_alert("Chardev: CD_IOC_WRITE_STRING: kern_buf %s\n", kern_buf);
                        break;

                case CD_IOC_REVERSE_STRING:
                        rv = string_rev(kern_buf);
                        pr_alert("Chardev: CD_IOC_REVERSE_STRING: kern_buf %s\n", kern_buf);
                        break;

                case CD_IOC_READ_STRING:
                        copy_to_user((char *) arg, kern_buf, buf_size);
                        break;
		
		case CD_IOC_FREE_BUF:
                        kfree(file);
                        break;

                case CD_IOC_READ_WRITE_REVERSE_STRING:
                        copy_from_user(&tmp_s, (struct string *) arg, sizeof(tmp_s));
                        pr_alert("Chardev: String from user space %s\n", tmp_s.original);
                        rv = string_rev(tmp_s.original);
                        pr_alert("Chardev: Reversed String %s\n", tmp_s.original);
                        copy_to_user(&(((struct string *) arg)->reverse), tmp_s.original, sizeof(tmp_s.original));
                        break;
                
		case CD_CREATE_KTHREAD:
		        /* Create the kernel thread with name 'mykthread' */
		        thread_st = kthread_run(thread_fn, NULL, "mykthread");
		        if (IS_ERR(thread_st)) {
                		pr_info("Chardev: Thread creation failed: %ld\n", PTR_ERR(thread_st));	
				thread_st = NULL;
			}
			pr_info("Chardev: thread PID = %i :PPID = %i :State = %ld\n", thread_st->pid, thread_st->parent->pid, thread_st->state);
			break;
                
		case CD_REMOVE_KTHREAD:
			/* if you are sending signal to kill thread then do not use this ioctl*/
			if (thread_st != NULL) {
				kthread_stop(thread_st);
				pr_info("Chardev: Thread stopped");
			}
			break;

	}

	return 0;
}

static int dev_open(struct inode *inodep, struct file *filep)
{
	if(!mutex_trylock(&ledchar_mutex)) {
		pr_alert("Chardev: Device in use by another process");
		return -EBUSY;
	}

	numberopens++;
	pr_info("Chardev: Device has been opened %d time(s)\n", numberopens);

	return 0;
}
 
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
	int ret = 0;
	
	/* copy_to_user has the format ( * to, *from, size) and returns 0 on success */
	ret = copy_to_user(buffer, message, size_of_message);
 
	if (ret == 0) {
		pr_info("Chardev: Sent %d characters to the user\n", size_of_message);
		return (size_of_message);
	}
	else {
		pr_info("Chardev: Failed to send %d characters to the user\n", ret);
		/* Failed -- return a bad address message (i.e. -14) */
		return -EFAULT;
	}

}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
	/* appending received string with its length */
	sprintf(message, "%s(%zu letters)", buffer, len);

	/* store the length of the stored message */
	size_of_message = strlen(message);
	pr_info("Chardev: Received %zu characters from the user\n", len);
	
	return len;
}
 
static int dev_release(struct inode *inodep, struct file *filep)
{
	pr_info("Chardev: Device successfully closed\n");
	
	mutex_unlock(&ledchar_mutex);
	
	return 0;
}

static struct file_operations fops =
{
	.open = dev_open,
	.read = dev_read,
	.write = dev_write,
	.release = dev_release,
	.unlocked_ioctl = dev_ioctl,
};

static int __init gpio_init(void)
{
	int ret = 0;

	pr_info("Chardev: %s: loading\n", __func__);

	/* Try to dynamically allocate a major number for the device -- more difficult but worth it */
	majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
	if (majorNumber<0) {
		pr_alert("Chardev: Chardev failed to register a major number\n");
		return majorNumber;
	}
	pr_info("Chardev: registered correctly with major number %d\n", majorNumber);
 
	/* Register the device class */
	ledcharClass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(ledcharClass)) {
		unregister_chrdev(majorNumber, DEVICE_NAME);
		pr_alert("Chardev: Failed to register device class\n");
		
		/* Correct way to return an error on a pointer */
		return PTR_ERR(ledcharClass);
	}
	pr_info("Chardev: device class registered correctly\n");
 
	/* Register the device driver */
	ledcharDevice = device_create(ledcharClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
	if (IS_ERR(ledcharDevice)) {
		class_destroy(ledcharClass);
		unregister_chrdev(majorNumber, DEVICE_NAME);
		pr_alert("Chardev: Failed to create the device\n");
		return PTR_ERR(ledcharDevice);
	}

	pr_info("Chardev: device class created correctly\n");

	mutex_init(&ledchar_mutex);

	return ret;
}

static void __exit gpio_exit(void)
{

	device_destroy(ledcharClass, MKDEV(majorNumber, 0));
	class_unregister(ledcharClass);
	class_destroy(ledcharClass);
	unregister_chrdev(majorNumber, DEVICE_NAME);
	
	mutex_destroy(&ledchar_mutex);
	
	pr_info("Chardev: %s: removed\n", __func__);
}

module_init(gpio_init);
module_exit(gpio_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Dharmender Sharma");
MODULE_DESCRIPTION("simpli-fi-dev-gpio driver");
MODULE_VERSION("1.0");
