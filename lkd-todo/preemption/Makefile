obj-m   := kp.o
KDIR    := /lib/modules/$(shell uname -r)/build
PWD 	:= $(shell pwd) 
  
all: 
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
clean:
	rm -rf *.o *.ko *.mod.c .*.cmd modules.order Module.symvers
