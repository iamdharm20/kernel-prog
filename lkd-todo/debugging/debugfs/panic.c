/*
It will happen eventually, so you might as well learn do deal with it.
TODO: how to scroll up to see full trace? Shift + Page Up does not work as it normally does:
https://superuser.com/questions/848412/scrolling-up-the-failed-screen-with-kernel-panic
The alternative is to get the serial data out streamed to console or to a file:
- https://superuser.com/questions/269228/write-qemu-booting-virtual-machine-output-to-a-file
- http://www.reactos.org/wiki/QEMU#Redirect_to_a_file
*/

#include <linux/module.h>
#include <linux/kernel.h>

int init_module(void)
{
	printk(KERN_INFO "panic init\n");
	panic("hello panic");
	return 0;
}

void cleanup_module(void)
{
	printk(KERN_INFO "panic cleanup\n");
}
