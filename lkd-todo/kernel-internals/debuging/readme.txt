http://lkw.readthedocs.io/en/latest/doc/06_kernel_bug_reporting.html

Kernel has built-in functions/macros for BUGS

BUG(), BUG_ON(), dump_stack(), panic() example

make
make insert
dmesg

dump_stack 	: echo 3 > /proc/my_bugon_driver
BUG 		: echo 1 > /proc/my_bugon_driver
panic 		: echo 4 > /proc/my_bugon_driver
BUG_ON 		: echo 4 > /proc/my_bugon_driver
