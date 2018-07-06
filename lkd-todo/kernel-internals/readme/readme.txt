make
sudo insmod mmap_kernel.ko
mount -t debugfs nodev /sys/kernel/debug/
./mmap_user
sudo rmmod mmap_kernel.ko
