Stupid kernel module for displaying the virtual address of `sys_call_table'.


HOW-TO
	# To compile the module, use the Makefile. After a successful
	# compilation, there should be a `.ko' file created.
	$ make
	
	# To check details on the newly created kernel module.
	$ modinfo syscall_table_addr.ko

	# To insert the module, use `insmod' command.
	$ insmod syscall_table_addr.ko
	
	# The output is written to the kernel ring buffer.
	$ tail -f /var/log/dmesg
	# Alternatively, you can just run `dmesg'.

	# To remove the module, use `rmmod'.
	$ rmmod syscall_table_addr.ko
