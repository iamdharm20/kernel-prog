
/*
Timers are callbacks that run when an interrupt happens, from the interrupt context itself.
Therefore they produce more accurate timing than thread scheduling, which is more complex,
but you can't do too much work inside of them.
See also:
- http://stackoverflow.com/questions/10812858/timers-in-linux-device-drivers
- https://gist.github.com/yagihiro/310149
*/

#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/timer.h>

#if 0
unsigned long j, stamp_1, stamp_half, stamp_n;
j = jiffies; /* read the current value */
stamp_1 = j + HZ; /* 1 second in the future */
stamp_half = j + HZ/2; /* half a second */
stamp_n = j + n * HZ / 1000; /* n milliseconds */
#endif

MODULE_LICENSE("GPL");

static void callback(unsigned long data);

static unsigned long onesec;

DEFINE_TIMER(mytimer, callback, 0, 0);

static void callback(unsigned long data)
{
	unsigned long j = jiffies;
	pr_info("%u\n", (unsigned)jiffies);

	mod_timer(&mytimer, jiffies + onesec);

	pr_info("mykmod timer %u jiffies\n", (unsigned)j);
}

static int myinit(void)
{
	unsigned long j = jiffies;
	onesec = msecs_to_jiffies(1000 * 1);

	pr_info("mykmod loaded %u/%u jiffies before\n", (unsigned)j, (unsigned)onesec);

	/* Changes the expiration time of an already scheduled timer structure. It can also
	act as an alternative to add_timer.
	add_timer:Registers the timer structure to run on the current CPU.
	*/
	mod_timer(&mytimer, jiffies + onesec);

	pr_info("mykmod loaded %u jiffies after \n", (unsigned)j);
	
	return 0;
}

static void myexit(void)
{
	del_timer(&mytimer);
	pr_info("mykmod exit\n");
}

module_init(myinit)
module_exit(myexit)
