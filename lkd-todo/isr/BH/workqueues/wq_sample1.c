/*
Usage:
	insmod /workqueue.ko
	# dmesg => worker
	rmmod workqueue
Creates a separate thread. So init_module can return, but some work will still get done.
Can't call this just workqueue.c because there is already a built-in with that name:
https://unix.stackexchange.com/questions/364956/how-can-insmod-fail-with-kernel-module-is-already-loaded-even-is-lsmod-does-not
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/workqueue.h>

static void work_handler(struct work_struct *w);

static struct workqueue_struct *wq = 0;

/* Statically create a work queue structure i.e during the compile time 
* DECLARE(_DELAYED)_WORK(name, void (*function)(struct work_struct *work)); 
*/
static DECLARE_DELAYED_WORK(work, work_handler);

static unsigned long onesec;

static void work_handler(struct work_struct *w)
{
	printk(KERN_INFO "worker\n");
        pr_info("mykmod work %u jiffies\n", (unsigned)onesec);
}


static int __init mykmod_init(void)
{
        onesec = msecs_to_jiffies(1000);
        pr_info("mykmod loaded %u jiffies\n", (unsigned)onesec);

        if (!wq)
                wq = create_singlethread_workqueue("myworkqueue");
	/* Queue or add work to a given worker thread:
           int queue_work(struct workqueue_struct *wq, struct work_struct *work)
           bool queue_delayed_work(struct workqueue_struct *wq, struct delayed_work *dwork, unsigned long delay);
           work will be added to the queue only after the delay 
	*/
        if (wq)
                queue_delayed_work(wq, &work, onesec);

        return 0;
}

static void __exit mykmod_exit(void)
{
	/* TODO why is this needed? Why flush_workqueue doesn't work? (re-insmod panics)
	 * http://stackoverflow.com/questions/37216038/whats-the-difference-between-flush-delayed-work-and-cancel-delayed-work-sync */
	/* flush_workqueue(queue); cancel_work_sync(&work); */
        if (wq)
                destroy_workqueue(wq);
        pr_info("mykmod exit\n");
}

module_init(mykmod_init);
module_exit(mykmod_exit);

MODULE_DESCRIPTION("mykmod");
MODULE_LICENSE("GPL");
