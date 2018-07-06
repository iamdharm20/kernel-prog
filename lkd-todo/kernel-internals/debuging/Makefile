#export CROSS_COMPILE=arm-linux-gnueabihf-
#export ARCH=arm
#KERNELDIR ?= ${HOME}/gitweb/artik533s-simplifi/linux-artik

obj-m := bug.o 

INSTALL_DIR=modules
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE)  -C $(KERNELDIR) M=$(PWD) modules
	@rm -rf ${INSTALL_DIR}
	@mkdir ${INSTALL_DIR}
	@mv -f *.o *.ko *.mod.c .*.cmd ${INSTALL_DIR}

modules_install:
	@$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

clean:
	rm -rf ${INSTALL_DIR} modules.order Module.symvers

insert: modules
	sudo dmesg -c
	sudo insmod modules/bug.ko

remove: clean
	sudo rmmod bug.ko
