/*
 * https://davejingtian.org/2014/07/09/kt-use-kernel-timers-in-the-linux-kernel/
 * Kernel module using the kernel timer
 * Reference: http://lwn.net/images/pdf/LDD3/ch07.pdf
 * http://davejingtian.org
 */
/*
Compared to start a timer directly in certain kernel thread which will make the thread sleep for a while, 
using a kernel timer may be much more desired without stopping the current working thread.
A kernel timer is a data structure that instructs the kernel to execute a user-defined function with a
user-defined argument at a user-defined time.“. One can imagine a real kernel timer worker thread maintaining 
its timer queue for a certain CPU and doing the work accordingly. However, kernel timers are not free to use. 

No access to user space is allowed. Because there is no process context, there is no path to the user 
space associated with any particular process.
The current pointer is not meaningful in atomic mode and cannot be used since the relevant code has no connection 
with the process that has been interrupted.
No sleeping or scheduling may be performed. Atomic code may not call schedule or a form of wait_event, 
nor may it call any other function that could sleep. For example, calling kmalloc(…, GFP_KERNEL) is against 
the rules. Semaphores also must not be used since they can sleep. 

#include <linux/timer.h>
struct timer_list {
        unsigned long expires;
        void (*function)(unsigned long);
        unsigned long data;
};

void init_timer(struct timer_list *timer);
struct timer_list TIMER_INITIALIZER(_function, _expires, _data);

void add_timer(struct timer_list * timer);
int del_timer(struct timer_list * timer);

*/

#include <linux/module.h>
#include <linux/timer.h>
#include <linux/spinlock.h>
 
/* A global struct acting as critical section */
struct global_data {
    /* Spinlock */
    spinlock_t lock;
    /* Counter */
    int count;
};
struct global_data kt_global_data;
 
/* A timer list */
struct timer_list kt_timer;
 
/* Timer callback */
void timer_callback(unsigned long arg)
{
    struct global_data *data = (struct global_data *)arg;
    printk(KERN_INFO "Entering: %s\n", __FUNCTION__);
 
    spin_lock(&(data->lock));
    data->count++;
    spin_unlock(&(data->lock));
    mod_timer(&kt_timer, jiffies + 10*HZ); /* restarting timer */
}
 
/* Init the timer */
static void kt_init_timer(void)
{
    init_timer(&kt_timer);
    kt_timer.function = timer_callback;
    kt_timer.data = (unsigned long)(&kt_global_data);
    kt_timer.expires = jiffies + 10*HZ; /* 10 second */
    add_timer(&kt_timer); /* Starting the timer */
 
    printk(KERN_INFO "kt_timer is started\n");
}
 
/* The normal module worker */
static void kt_do_the_work(void)
{
    printk(KERN_INFO "Before %s, count = %d\n", __FUNCTION__,
        kt_global_data.count);
 
    spin_lock(&(kt_global_data.lock));
    kt_global_data.count++;
    spin_unlock(&(kt_global_data.lock));
 
    printk(KERN_INFO "After %s, count = %d\n", __FUNCTION__,
        kt_global_data.count);
}
 
static int __init kt_init(void)
{
    printk(KERN_INFO "Entering: %s\n", __FUNCTION__);
     
    /* Init the global data */
    memset(&kt_global_data, 0x0, sizeof(struct global_data));
 
    /* Init the spinlock */
    spin_lock_init(&(kt_global_data.lock));
 
    /* Init the timer */
    kt_init_timer();
 
    /* Do our job */
    kt_do_the_work();
 
    return 0;
}
 
static void __exit kt_exit(void)
{
    printk(KERN_INFO "exiting kt module\n");
 
    /* Delete the timer */
    del_timer_sync(&kt_timer);
 
    printk(KERN_INFO "kt_global_data.count = [%d]\n",
        kt_global_data.count);
}
 
module_init(kt_init);
module_exit(kt_exit);
 
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("KT Module");
