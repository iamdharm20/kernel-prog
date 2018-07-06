sudo insmod platform-button.ko

/* to triger the signal from kernel space to user space. 
* the process ledchar-ioctl will have to pass the pid of itself using ioctl to the kernel 
* the kernel will use this pid and will generate the signal to the user space process ledchar-ioctl
*/

sudo ./ledchar-ioctl

sudo tail -f /var/log/kern.log

find . -name "ledchar"
udevadm info -a -p /sys/class/ledc/ledchar
