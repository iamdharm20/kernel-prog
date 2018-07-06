#export CROSS_COMPILE=arm-linux-gnueabihf-
#export ARCH=arm

INSTALL_DIR=modules
ifneq (${KERNELRELEASE},)
	obj-m := network-driver.o netfilter.o platform-driver.o
	module-objs := network-driver.o netfilter.o platform-driver.o

else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
#        KERNELDIR ?= ${HOME}/gitweb/artik533s-simplifi/linux-artik
        PWD := $(shell pwd)
default:
	$(MAKE)  -C $(KERNELDIR) M=$(PWD) modules
	@rm -rf ${INSTALL_DIR}
	@mkdir ${INSTALL_DIR}
	@mv -f *.o *.ko *.mod.c .*.cmd ${INSTALL_DIR}
clean:
	rm -rf ${INSTALL_DIR} modules.order Module.symvers
endif
