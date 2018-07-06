sudo insmod modules/poll.ko
sudo mount -t debugfs none /sys/kernel/debug
sudo ./poll.out /sys/kernel/debug/lkmc_poll/f
