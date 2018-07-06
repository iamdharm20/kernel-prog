https://sysplay.in/blog/tag/wait-queues/

$ insmod wait.ko
Major Nr: 250
$ cat /dev/mychar0
Inside open
Inside read
Scheduling out

This gets our process blocked. Open another shell to wake up the process:

$ echo 'y' > /dev/mychar0
Inside open
Inside write
y
Inside close
Woken up
Inside close

