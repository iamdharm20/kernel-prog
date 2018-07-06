#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/delay.h>
#include<linux/workqueue.h>
#include<linux/wait.h>


/* declare a wait queue*/
static wait_queue_head_t my_wait_queue;

/* declare a work queue*/
struct work_struct workq;

int flag = 0;

/*
   Note: wait queue is the list of processes waiting for an event

   The simplest way to sleep in the Linux kernel is via a macro called 'wait_event'

      wait_event(queue, condition)
      wait_event_interruptible(queue, condition)
      wait_event_timeout(queue, condition, timeout)
      wait_event_interruptible_timeout(queue, condition, timeout)

 The basic function that wakes up sleeping processes is called 'wake_up'.

      Wakes up all the processes waiting on the queue   
      void wake_up(wait_queue_head_t *queue); 
     
      Wakes up only the processes performing the interruptible sleep
      void wake_up_interruptible(wait_queue_head_t *queue); 

*/

void my_workqueue_handler(struct work_struct *work)
{
	printk("WORK QUEUE: I'm just a timer to wake up the sleeping moudlue. \n");
	msleep(10000);  /* sleep */

	printk("WORK QUEUE: time up MODULE !! wake up !!!! \n");
	wake_up_interruptible(&my_wait_queue);
	flag = 1; /* what happens if this is not set to 1? */
}

static int waitqueue_init(void)
{
	printk("Wait queue example ....\n");

	/* initialize the work queue */
	INIT_WORK(&workq, my_workqueue_handler);
	
	printk("schedule_work to wake up the task which is in the waitqueue after 10s\n");
	schedule_work(&workq);

	/* initialize the WAIT QUEUE head dynamically */
	init_waitqueue_head(&my_wait_queue);

	/* The process is put to sleep (TASK_INTERRUPTIBLE) until the condition evaluates to true 
	or a signal is received. The condition is checked each time the waitqueue wq is woken up.
	wake_up has to be called after changing any variable that could change the result of the wait condition.
	The function will return -ERESTARTSYS if it was interrupted by a signal and 0 if condition evaluated 
	to true. 
	*/
	printk("MODULE: This task is goint to sleep....\n");
	wait_event_interruptible(my_wait_queue, flag != 0); 

	printk("MODULE: Wakeup Wakeup I am Waked up........\n");
	return 0;
}

static void __exit waitqueue_exit(void)
{
	printk("<1> Start to cleanup \n");
}

module_init(waitqueue_init);
module_exit(waitqueue_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A Simple Blocking IO device");
