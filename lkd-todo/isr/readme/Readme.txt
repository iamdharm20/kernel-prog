
There are many ways to schedule work in the linux kernel: 
	timers, isoftirq, tasklets, work queues, and kernel threads. What are the guidelines for when to use one vs another? 

Softirq, timer functions and tasklets cannot sleep as they run in interrupt contaxt so they cannot wait on mutexes, condition variables, etc

Work queues and kernel thread run in process context so they are schedulable and can therefore sleep.

Kernel timers are good when you know exactly when you want something to happen, and do not want to interrupt/block a process in the meantime. 
They run outside process context, and they are also asynchronous with regard to other code, so they're the source of race conditions 
if you're not careful.

when we should use kthread and work-queues ?

A thread is yours to run for as long as you want. It doesn't have to return to some caller in order to do other work, 
	so you can put it in a loop (and that is usually done). The loop can contain arbitrary sleeps.

workqueue is used in a very specific scenario : when you want to delay your work for some later time. one of the situation could be when you are 
in interrupt handler and wants to come out as soon as possible. So with workqueue you can schedule your work for some later time 
while normal kthread can't be delayed.
