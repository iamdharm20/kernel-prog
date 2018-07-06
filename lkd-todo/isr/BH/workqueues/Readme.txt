https://www.kernel.org/doc/html/v4.10/core-api/workqueue.html
Work Queues: include/linux/workqueue.h, kernel/workqueue.c and kernel/workqueue_internal.h files
https://kukuruku.co/post/multitasking-in-the-linux-kernel-workqueues/

Workqueue is an asynchronous execution mechanism which is widely used across the kernel.
The main data structure associated with a work queue is a descriptor called workqueue_struct.
The work queue subsystem is an interface for creating kernel threads to handle work that is queued from elsewhere. 
	All of these kernel threads are called -- worker threads.
The Linux kernel provides special per-cpu threads that are called kworker.

The worklist field of the cpu_workqueue_struct structure is the head of a doubly linked list collecting the pending functions of the work queue. 
Every pending function is represented by a work_struct data structure.

The design :
	Work item 	: a simple struct that holds a pointer to the function handler that is to be executed asynchronously. 
	Work queue 	: a queue of work items
	Worker threads 	: Special purpose threads that execute the functions off the queue, one after the other.
	Workerpools 	: A thread pool that is used to manage the worker threads

Data structure:

	struct worker_pool;
	struct workqueue_struct;
	struct work_struct;

Work queue functions:

The create_workqueue("foo" ) function receives as its parameter a string of characters and returns the address of a workqueue_struct descriptor 
for the newly created work queue. The function also creates n worker threads (where n is the number of CPUs effectively present in the system), 
named after the string passed to the function: foo/0, foo/1, and so on. 
The create_singlethread_workqueue( ) function is similar, but it creates just one worker thread, no matter what the number of CPUs in the system is. 
To destroy a work queue the kernel invokes the destroy_workqueue( ) function, which receives as its parameter a pointer to a workqueue_struct array.

Statically create a work queue structure: /* during the compile time */
	DECLARE_WORK(name, function, data)
	DECLARE(_DELAYED)_WORK(name, void (*function)(struct work_struct *work));

Dynamically initialize a work queue structure:
	INIT_WORK(p, function, data)

Create a new worker thread:
	old: struct workqueue_struct *create_workqueue(const char *name)
	struct workqueue_struct *create_singlethread_workqueue(const char *name);

	create_workqueue() will have one worker thread for each CPU on the system; 
	create_singlethread_workqueue(), instead, creates a workqueue with a single worker process.
	The name of the queue is limited to ten characters; ( ps or top). 

	new: int alloc_workqueue(char *name, unsigned int flags, int max_active);
	The name parameter names the queue, but, unlike in the older implementation, it does not create threads using that name. 
	The flags parameter selects among a number of relatively complex options on how work submitted to the queue will be executed; 
	its value can include:

	WQ_NON_REENTRANT: "classic" workqueues guaranteed that no task would be run by two threads simultaneously on the same CPU, 
	but made no such guarantee across multiple CPUs. If it was necessary to ensure that a task could not be run simultaneously anywhere 
	in the system, a single-threaded workqueue had to be used, possibly limiting concurrency more than desired. With this flag, 
	the workqueue code will provide that systemwide guarantee while still allowing different tasks to run concurrently.

	WQ_UNBOUND: workqueues were designed to run tasks on the CPU where they were submitted in the hope that better memory cache behavior 
	would result. This flag turns off that behavior, allowing submitted tasks to be run on any CPU in the system. It is intended for situations 
	where the tasks can run for a long time, to the point that it's better to let the scheduler manage their location. 
	Currently the only user is the object processing code in the FS-Cache subsystem.

	WQ_FREEZEABLE: this workqueue will be frozen when the system is suspended. Clearly, workqueues which can run tasks as part of the 
	suspend/resume process should not have this flag set.

	WQ_RESCUER: this flag marks workqueues which may be involved in memory reclaim; the workqueue code responds by ensuring that there is always 
	a thread available to run tasks on this queue. It is used, for example, in the ATA driver code, which always needs to be able to run 
	its I/O completion routines to be sure it can free memory.

	WQ_HIGHPRI: tasks submitted to this workqueue will put at the head of the queue and run (almost) immediately. Unlike ordinary tasks, 
	high-priority tasks do not wait for the CPU to become available; they will be run right away. That means that multiple tasks submitted 
	to a high-priority queue may contend with each other for the processor.

	WQ_CPU_INTENSIVE: tasks on this workqueue can be expected to use a fair amount of CPU time. To keep those tasks from delaying the execution 
	of other workqueue tasks, they will not be taken into account when the workqueue code determines whether the CPU is available or not. 
	CPU-intensive tasks will still be delayed themselves, though, if other tasks are already making use of the CPU. 

	The combination of the WQ_HIGHPRI and WQ_CPU_INTENSIVE flags takes this workqueue out of the concurrency management regime entirely. 
	Any tasks submitted to such a workqueue will simply run as soon as the CPU is available.

	The final argument to alloc_workqueue() (we are still talking about alloc_workqueue(), after all) is max_active. This parameter limits 
	the number of tasks which can be executing simultaneously from this workqueue on any given CPU. The default value 
	(used if max_active is passed as zero) is 256, but the actual maximum is likely to be far lower, given that the workqueue code really 
	only wants one task using the CPU at any given time. Code which requires that workqueue tasks be executed in the order in which they 
	are submitted can use a WQ_UNBOUND workqueue with max_active set to one

Destroy a worker thread:
	void destroy_workqueue(struct workqueue_struct *wq)

Queue or add work to a given worker thread:
	int queue_work(struct workqueue_struct *wq, struct work_struct *work)
	bool queue_delayed_work(struct workqueue_struct *wq, struct delayed_work *dwork, unsigned long delay); 
	/* work will be added to the queue only after the delay */

Queue work, after the given delay, to the given worker thread:
	int queue_delayed_work(struct workqueue_struct *wq, struct work_struct *work, unsigned long delay)

Wait on all pending work on the given worker thread:
	void flush_workqueue(struct workqueue_struct *wq)
	/* Force Quit */
	bool flush_work(struct work_struct *work);
	bool flush_delayed_work(struct delayed_work *dwork);

Schedule work to the default worker thread:
	int schedule_work(struct work_struct *work) or queue_work(keventd_wq,w);

Schedule work, after a given delay, to the default worker thread:
	int schedule_delayed_work(struct work_struct *work, unsigned long delay) or queue_delayed_work(keventd_wq,w,d) (on any CPU);

Wait on all pending work on the default worker thread:
	void flush_scheduled_work(void) or flush_workqueue(keventd_wq);

Cancel the given delayed work:
	int cancel_delayed_work(struct work_struct *work)

schedule_delayed_work_on(cpu,w,d) or queue_delayed_work(keventd_wq,w,d) (on a given CPU)

