#include <linux/init.h>
#include <linux/kernel.h>           //used for do_exit()
#include <linux/module.h>       /* Specifically, a module */
#include <linux/delay.h>        /* for msleep() */
#include <asm/siginfo.h>        /* for signals */
#include <linux/signal.h>
#include <linux/kmod.h>
#include <linux/threads.h>          //used for allow_signal
#include <linux/kthread.h>          //used for kthread_create
#include <linux/delay.h> 	    //used for ssleep()

MODULE_DESCRIPTION("sending signal from 1 kthread to another"
                   " and using default signal handler");

//Follow me on twitter
MODULE_AUTHOR("@Kashish_Bhatia");

struct task_struct *k2;
struct task_struct *saved_task;

struct siginfo data_for_handler;

//signal descriptor
struct sigaction sig;

//Global vars
int val = 0;

int signal_receive_cnt = 0;     //max # of times signal should be serviced

extern pid_t kthread_pid;
struct task_struct *target_kthread;
struct pid *pid_struct;

//Thread func for k2
int send_signal_to_task(void *arg)
{
        int ret;
        while (!kthread_should_stop()) {
                msleep_interruptible(2000);
                if (kthread_pid && signal_receive_cnt != 2) {
                        printk("%s : pid of kthread to which signal is sent = %u\n", current->comm, kthread_pid);
                        //TODO: siginfo is passed to default sighandler.
                        //Can't check the info, as i dont have access to default
                        //signal handler.
                        data_for_handler.si_code = SI_KERNEL;
                        data_for_handler.si_int = val;
                        printk("%s : sending signal to process with pid %u...\n", current->comm, kthread_pid);

                        //TODO: convert pid to task_struct
                        pid_struct = find_get_pid(kthread_pid);
                        target_kthread = pid_task(pid_struct, PIDTYPE_PID);

                        //send signal
                        ret = send_sig_info(SIGUSR1, &data_for_handler, target_kthread);
                        if (ret) {
                                printk("%s : Error sending signal to thread:%s with pid:%d, status %d\n",
                                        current->comm, saved_task->comm, saved_task->pid, ret);
                                //return ret;
                        } else {
                                signal_receive_cnt++;
                        }
                }
        }
        printk("%s : exiting \"%s\"\n", current->comm, __FUNCTION__);
        return 0;
}

int __init startup(void)
{
        printk(KERN_ALERT "send_signal module is starting\n");
        k2= kthread_run(send_signal_to_task, NULL, "K2");
        return 0;
}

void __exit cleanup(void)
{
        printk("cleaning module \n");
        kthread_stop(k2);
        printk("stopped kernel thread 2 \n");
}

module_init(startup);
module_exit(cleanup);
MODULE_LICENSE("GPL");
