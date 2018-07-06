chardev and polling steps:
https://stackoverflow.com/questions/34027366/implementing-poll-in-a-linux-kernel-module
https://stackoverflow.com/questions/30035776/how-to-add-poll-function-to-the-kernel-module-code
http://playopensuse.blogspot.in/2015/04/workqueue-mechanism-in-linux.html
https://blackfin.uclinux.org/doku.php?id=linux-kernel:polling
https://blackfin.uclinux.org/doku.php?id=linux-kernel:ioctls&s[]=ioctl
https://blackfin.uclinux.org/doku.php?id=mmap&s[]=mmap
https://blackfin.uclinux.org/doku.php?id=user_space_memory_allocation&s[]=mmap
https://blackfin.uclinux.org/doku.php?id=advanced_device_driver_topics&s[]=mmap
 
dharm@dharm:~/gitweb/LKD/driver/platfrom$ ls -al /dev/chardev 
crw------- 1 root root 243, 0 Sep 18 23:13 /dev/chardev

dharm@dharm:~/gitweb/LKD/driver/platfrom$ ls -al /sys/class/chardev/chardev/
total 0
drwxr-xr-x 3 root root    0 Sep 18 23:14 .
drwxr-xr-x 3 root root    0 Sep 18 23:14 ..
-rw-r--r-- 1 root root 4096 Sep 18 23:14 chardev_file
-r--r--r-- 1 root root 4096 Sep 18 23:14 dev
drwxr-xr-x 2 root root    0 Sep 18 23:14 power
lrwxrwxrwx 1 root root    0 Sep 18 23:14 subsystem -> ../../../../class/chardev
-rw-r--r-- 1 root root 4096 Sep 18 23:14 uevent

dharm@dharm:/sys$ sudo find . -name "chardev"
./devices/virtual/chardev
./devices/virtual/chardev/chardev
./class/chardev
./class/chardev/chardev
./module/chardev

dharm@dharm:/$ udevadm info -a -p /sys/class/chardev/chardev 

Udevadm info starts with the device specified by the devpath and then
walks up the chain of parent devices. It prints for every device
found, all possible attributes in the udev rules key format.
A rule to match, can be composed by the attributes of the device
and the attributes from one single parent device.

  looking at device '/devices/virtual/chardev/chardev':
    KERNEL=="chardev"
    SUBSYSTEM=="chardev"
    DRIVER==""
    ATTR{chardev_file}=="0

make
sudo insmod poll_module.ko
make poll_app 
sudo ./poll_app /sys/kernel/debug/lkmc_poll/f

sudo insmod chardev.ko
sudo tail -f /var/log/kern.log
make chardev_test
sudo strace ./chardev_test
sudo strace ./poll_app /dev/chardev
sudo cat /sys/class/chardev/chardev/chardev_file

mmap:
http://www.compsoc.man.ac.uk/~moz/kernelnewbies/code/mmap/index.php3
https://coherentmusings.wordpress.com/2014/06/10/implementing-mmap-for-transferring-data-from-user-space-to-kernel-space/
http://developer.axis.com/wiki/doku.php%3Fid=mmap.html
