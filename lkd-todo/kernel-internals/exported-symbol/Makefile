#export CROSS_COMPILE=arm-linux-gnueabihf-
#export ARCH=arm
#KERNELDIR ?= ${HOME}/gitweb/artik533s-simplifi/linux-artik

#Note : know the diff b/w += and := 
#obj-m += signal_kernel.o
#obj-m := seqfile.o procfs.o debugfs.o sysfs_ex2.o sysfs_ex.o usermodehelper.c
obj-m := module1.o module2.o our_glob_syms.o module_param.o 

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
