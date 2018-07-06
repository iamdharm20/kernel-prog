#export CROSS_COMPILE=arm-linux-gnueabihf-
#export ARCH=arm
#KERNELDIR ?= ${HOME}/gitweb/artik533s-simplifi/linux-artik

#Note : know the diff b/w += and := 
#obj-m += signal_kernel.o
#obj-m := seqfile.o procfs.o debugfs.o sysfs_ex2.o sysfs_ex.o usermodehelper.c
obj-m := platform-driver.o platform-driver-1.o ioctl.o mmap_kernel.o mmap_kernel_cdev.o poll_kernel.o signal_kernel.o \
	 debugfs.o cdev.o usermodehelper.o seqfile.o proc.o mtest.o tasklet.o koject.o klinklist.o kparam.o 

INSTALL_DIR=modules
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE)  -C $(KERNELDIR) M=$(PWD) modules
	@rm -rf ${INSTALL_DIR}
	@mkdir ${INSTALL_DIR}
	@mv -f *.o *.ko *.mod.c .*.cmd ${INSTALL_DIR}

clean:
	rm -rf ${INSTALL_DIR} modules.order Module.symvers
