#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/sched.h>

char tasklet_data[] = "my_tasklet_function_was_called";

struct tasklet_struct *task;

void tasklet_fun(unsigned long data)
{

	printk("%s\n", (char *)data);
	
	if(in_interrupt())
        	printk(KERN_INFO "Running as an interrupt contaxt");
	else
        	printk(KERN_INFO "Running as a process contaxt");
	
	printk("\n The state is %ld", task->state);
	printk("\n The count is %lx", task->count);

	return;
}

//DECLARE_TASKLET(my_tasklet, my_tasklet_function, (unsigned long)&my_tasklet_data);

int init_module(void)
{
	task  = kmalloc(sizeof(struct tasklet_struct),GFP_KERNEL);
	tasklet_init(task, tasklet_fun, (unsigned long)&tasklet_data);
	
	/* https://stackoverflow.com/questions/39876457/how-to-sleep-in-the-linux-kernel-space : sleep*/

	struct task_struct *process = current; // getting global current pointer
	//struct task_struct *curr = get_current();
	
	printk(KERN_NOTICE "assignment: current process: %s, PID: %d", process->comm, process->pid);

	do {
		process = process->parent;
		printk(KERN_NOTICE "assignment: parent process: %s, PID: %d", process->comm, process->pid);
	} while (process->pid != 0);

	msleep(5024);
	tasklet_schedule(task);
	return 0;
}

void cleanup_module(void)
{
	tasklet_kill(task);
	return;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SysPlay Workshops <workshop@sysplay.in>");
MODULE_DESCRIPTION("Tasklet Demo");
