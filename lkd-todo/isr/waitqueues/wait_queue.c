#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
#include <linux/wait.h>
#include <linux/sched.h> 	/* wake_up_process() */
#include <linux/kthread.h> 	/* kthread_create(), kthread_run() */
#include <linux/delay.h>
#include <linux/err.h>		/* IS_ERR(), PTR_ERR() */

static struct task_struct *thread_st;
static DECLARE_WAIT_QUEUE_HEAD(wq);
static int flag = 0;

#if 0
int threadfunc(void *data){
        …
        while(1){ 
               set_current_state(TASK_UNINTERRUPTIBLE); 
               if(kthread_should_stop()) break;
               if(){//The condition is true
                      //Business processing
               }
               else{//The condition is false
                      //Let CPU run other threads, and re scheduled within a specified period of time
                      schedule_timeout(HZ);
               }
        }
        …
        return 0;
}

in cleanup function:
            if(test_task){
                kthread_stop(test_task);
                test_task = NULL;
            } 
#endif

static int thread_fn(void *unused)
{
	printk("Going to Sleep\n");
	wait_event_interruptible(wq, flag != 0);
	flag = 0;
	printk("Woken Up\n");
	thread_st = NULL;
	do_exit(0i);
}


static int __init init_thread(void)
{
	int err = 0;

	printk(KERN_INFO "Creating Thread 1\n");

	/* kthread_run => kthread_create + wake_up_process */
	thread_st = kthread_run(thread_fn, NULL, "mythread");
	if(IS_ERR(thread_st)){ 
		printk("Unable to start kernel thread.\n");
		err = PTR_ERR(thread_st);
		thread_st = NULL;
		return err;
	}
	printk("kthread pid : %d state:%ld\n", thread_st->pid, thread_st->state);
	printk("on_cpu: %d\n", thread_st->on_cpu);

	ssleep(10);
	printk("Waking Up Thread\n");
	flag = 1;
	wake_up_interruptible(&wq);
	return 0;
}

static void __exit cleanup_thread(void)
{
	printk("Cleaning Up\n");
	if (thread_st != NULL)
	{
		kthread_stop(thread_st);
		printk(KERN_INFO "Thread stopped");
	}
}

module_init(init_thread);
module_exit(cleanup_thread);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SysPlay Workshops <workshop@sysplay.in>");
MODULE_DESCRIPTION("Wait Queue Demo");
