/*
we generate poll events from a separate thread. In real life, poll events will likely be triggered by interrupts, 
when the hardware has finished some job, and new data became available for userland to read.

TODO now yet waiting for anything!
Basic poll file_operation example.
Waits for a second, give jiffies to user, wait for a second...

Referance example:

    drivers/char/rtc.c
    fs/proc/kmsg.c
    drivers/char/random.c

insmod /poll.ko
mount -t debugfs none /sys/kernel/debug
/poll.out /sys/kernel/debug/lkmc_poll/f

What exactly happens is that on calling poll_wait() the kernel calls all the fops->poll on all associated fds,
passing the the global poll table.
poll_wait() adds the process' wait_queue(events its is waiting for) to this global poll table. 
Or simply,a process adds itself to all the wait queues it is dependent on, and on poll/poll_wait invocation 
the kernel checks the global poll table if the event has occured. If the event is not ready 
(blocked on I/O, device not ready etc), it puts the process to sleep. Once an event has occurred (say read on FD),
it is removed from all wait queues and all the wake handlers associated with the queue are called waking up the process. 
So from the outside it'll look like the poll_wait() block until an event has occured,
but actually the event triggers the wakeup of the process. 
*/

#include <asm/uaccess.h> /* copy_from_user, copy_to_user */
#include <linux/debugfs.h>
#include <linux/delay.h> /* usleep_range */
#include <linux/errno.h> /* EFAULT */
#include <linux/fs.h>
#include <linux/jiffies.h>
#include <linux/kernel.h> /* min */
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/poll.h>
#include <linux/printk.h> /* printk */
#include <linux/wait.h> /* wait_queue_head_t, wait_event_interruptible, wake_up_interruptible  */

static char readbuf[1024];
static size_t readbuflen;

static struct dentry *dir;
static struct task_struct *kthread;
static wait_queue_head_t waitqueue;

static ssize_t read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
	ssize_t ret;

	if (copy_to_user(buf, readbuf, readbuflen)) {
		ret = -EFAULT;
	} else {
		 ret = readbuflen;
	}
	readbuflen = 0;
	return ret;
}
/*
If you return 0 here, then the kernel will sleep until an event happens in the queue.
This gets called again every time an event happens in the wait queue.
*/
unsigned int poll(struct file *filp, struct poll_table_struct *wait)
{
	/*TODO*/
	/*wait_event_interruptible(waitqueue, (dev->rp != dev->wp));*/
	pr_info("poll_wait before\n");
	poll_wait(filp, &waitqueue, wait);
	pr_info("poll_wait after\n");
	/* return POLLIN | POLLRDNORM if you have some new data to read, and 0 in case there is no new data to read */
	if (readbuflen)
		return POLLIN | POLLRDNORM;
	else
		return 0;

}

static int kthread_func(void *data)
{
	while (!kthread_should_stop()) {
		readbuflen = snprintf(readbuf, sizeof(readbuf), "%llu", (unsigned long long)jiffies);
		usleep_range(1000000, 1000001);
		/* Notify your waitqueue once you have new data */
	//	wake_up_interruptible(&waitqueue);
		wake_up(&waitqueue);
	}
	return 0;
}

static const struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = read,
	.poll = poll,
};

static int myinit(void)
{
	dir = debugfs_create_dir("lkmc_poll", 0);
	debugfs_create_file("f", 0666, dir, NULL, &fops);
	
	init_waitqueue_head(&waitqueue);
	
	kthread = kthread_create(kthread_func, NULL, "mykthread");
	wake_up_process(kthread);
	
	return 0;
}

static void myexit(void)
{
	debugfs_remove_recursive(dir);
}

module_init(myinit);
module_exit(myexit);

MODULE_LICENSE("GPL");

