
#include <linux/init.h>
#include <linux/module.h>       /* Specifically, a module */
#include <linux/kernel.h>       /* We're doing kernel work */
#include <linux/kthread.h>
#include <linux/delay.h>        /* for msleep() */
#include <asm/siginfo.h>        /* for signals */
#include <linux/signal.h>
#include <linux/kmod.h>         /* for request_module() */

MODULE_DESCRIPTION("dynamically loading module through kmod subsystem and "
                   "receiving signal from another module");

struct task_struct *k1;
struct task_struct *saved_task;

//Global vars
int val = 0;
int flag = 0;                   // a flag is set when val reached 10

pid_t kthread_pid;

//Thread func for k1
int inc_func(void *arg)
{
        int ret;
        saved_task = current;
        printk("%s : pid of current thread %u\n", current->comm, current->pid);
        kthread_pid = current->pid;

        msleep(1000);
        //load module "send_signal.ko"
        ret = request_module("send_signal");
        if (ret != 0)
                printk("%s : unable to load module, Status=%d\n", current->comm, ret);

        allow_signal(SIGUSR1);

        set_current_state(TASK_INTERRUPTIBLE);

        while (!kthread_should_stop()) {
                int ret;
                ret = schedule_timeout(msecs_to_jiffies(1000));

                if (signal_pending(current)) {
                        printk("%s : signal is pending****\n", current->comm);
                        flush_signals(current);
                }

                if (val <=10 && !flag) {
                        val++;
                        printk("%s : current value is %d\n", current->comm, val);
                        if (val == 10) {
                                printk("%s : setting flag\n", current->comm);
                                flag = 1;
                        }
                }
                set_current_state(TASK_INTERRUPTIBLE);
        }
        set_current_state(TASK_RUNNING);
        printk("%s : out of while loop, exiting \"%s\"\n", current->comm, __FUNCTION__);
        return 0;
}

int __init startup(void)
{
        k1 = kthread_run(inc_func, NULL, "K1");
        return 0;
}

void __exit cleanup(void)
{
        printk("cleaning module \n");
        kthread_stop(k1);
        printk("stopped kernel thread 1 \n");
}

module_init(startup);
module_exit(cleanup);
MODULE_LICENSE("GPL");
