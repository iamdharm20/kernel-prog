https://opensourceforu.com/2012/05/linux-device-drivers-module-interactions/
http://lkw.readthedocs.io/en/latest/doc/04_exporting_symbols.html

Exercises
write a kernel module to which we can pass a string and it does the following. 
It must have the functions exported so that another kernel module can use the functions. 
	#. Find the length of the string. mystring_find_length() 
	#. Returns the reverse of the string. mystring_get_reverse() 
	#. Returns the rotation of the string by 3 places.
	 char *mystring_get_rotated(char *srcstr, char *deststr, int rotations, int direction) 
	#. Returns if the string is palindrome or not. int mystring_is_palindrome(char *str) 
	#. Returns a character array where you have saved only the characters which are present in the even indexes. 
	#. Returns a string which has all the letter capitalized. #. Returns a string which has all the letter converted to lowercase.
For the above kernel module write a testcase module which will call the functions and test if the functions are working correctly.

EXPORT_SYMBOL(sym)
EXPORT_SYMBOL_GPL(sym)
EXPORT_SYMBOL_GPL_FUTURE(sym)

steps:
sudo insmod modules/module_param.ko
cat /proc/kallsyms | grep our_glob_syms


See the exported symbols:

	Module1 exports the symbols.
	The exported symbols and other functions in the kernel can be seen in the /proc/kallsyms file. Its is a huge file.
	Let us see the difference in the file after inserting the module1.ko.
	That file clearly that the functions print_hello() and others are from module1.
	The UpperCase T says that the functions are exported (available for others to use) while a lowercase says its not exported.

	Run the following commands to make two files with the list of symbols.
	cat /proc/kallsyms > /tmp/1
	
	Insert our module.
	sudo insmod modules/module2.ko
	sudo insmod modules/module1.ko

	modinfo imodules/module1.ko && modinfo modules/module2.ko && modinof modules/module_param.ko

	Module.order file. It has the order in which the modules should be loaded.
	cat modules.order

	Module.symvers file. It shows the symbols which are exported
	cat Module.symvers
	
	Save the symbols in another file.
	cat /proc/kallsyms > /tmp/2
	
	See the difference.
	diff /tmp/1 /tmp/2

	Tool modprobe:
	modprobe automatically loading all the modules.
	modprobe understands in which order the modules are to be loaded.
	First remove the modules.
	Run the command modprobe module2 loads both the module.

	Tool - depmod:
	DONT RUN IT. depmod is smart enough to find the dependencies and write to a file
	don’t run it as it will overwrite the original file. First take backup of the file.
        depmod ABSOLUTE_PATH_OF_THE_MODULE1 ABSOLUTE_PATH_OF_THE_MODULE2 see the file /modules/3.2.0-23-generic/modules.dep




steps:
sudo insmod modules/module_param.ko cfg_value=-10	
cat /sys/module/module_param/parameters/cfg_value
sudo su
echo 20 > /sys/module/module_param/parameters/cfg_value


Building the driver (module_param.ko file) using the usual driver Makefile
Loading the driver using insmod (with and without parameters)
Various experiments through the corresponding /sys entries
And finally, unloading the driver using rmmod.

Note the following:

Initial value (3) of cfg_value becomes its default value when insmod is done without any parameters.
Permission 0764 gives rwx to the user, rw- to the group, and r-- for the others on the file cfg_value under 
	the parameters of module_param under /sys/module/.

Check for yourself:

The output of dmesg/tail on every insmod and rmmod, for the printk outputs.
Try writing into the /sys/module/module_param/parameters/cfg_value file as a normal (non-root) user.

