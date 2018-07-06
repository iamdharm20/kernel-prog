/*
 * syscall_table_addr.c: Finds the virtual address of `sys_call_table'.
 *
 * Note: The returned value might differ from the value listed
 *	 in /boot/System.map-$(uname -r), but it maps to the 
 *	 same physical address.
 *	 Reference: Documentation/x86/x86_64/mm.txt (on kernel.org).
 */ 

#include <linux/init.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Srinidhi Kaushik");
MODULE_DESCRIPTION("Finds the address of `sys_call_table'.");


asmlinkage unsigned long **sys_call_table_addr;

static unsigned long **fetch_table_addr(void) {
	unsigned long offset;
	unsigned long **track;

	for(offset = PAGE_OFFSET; offset < ULLONG_MAX;
	    offset += sizeof(void *)) {
		track = (unsigned long **) offset;

		if(track[__NR_close] == (unsigned long *) sys_close)
			return track;
    	}
	
	return NULL;
}

static int __init syscall_table_addr_init(void) {
	printk(KERN_INFO "syscall_table_addr: Initialized.\n");
	sys_call_table_addr = fetch_table_addr();

	if (!sys_call_table_addr) {
		printk(KERN_ERR "syscall_table_addr: Unable to fetch the "
				"system call table!\n");
		return -EPERM;
	}
	
	printk(KERN_INFO "syscall_table_addr: The system call table is at "
			 "`0x%p'.\n", sys_call_table_addr);
	
	return 0;
}

static void __exit syscall_table_addr_exit(void) {
	printk(KERN_INFO "syscall_table_addr: Terminated.\n");
}

module_init(syscall_table_addr_init);
module_exit(syscall_table_addr_exit);
