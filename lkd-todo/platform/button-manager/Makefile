export CROSS_COMPILE=arm-linux-gnueabihf-
export ARCH=arm

obj-m := platform_button.o
module-objs := platform_button.o

INSTALL_DIR=modules
KERNELDIR ?= /home/dharm/gitweb/artik533s-simplifi/linux-artik/
PWD := $(shell pwd)

all:
	$(MAKE)  -C $(KERNELDIR) M=$(PWD) modules
	#make -C /home/dharm/gitweb/artik533s-simplifi/linux-artik/ M=/home/dharm/gitweb/test modules
	@rm -rf ${INSTALL_DIR}
	@mkdir ${INSTALL_DIR}
	@mv -f *.o *.ko *.mod.c .*.cmd ${INSTALL_DIR}

clean:
	rm -rf ${INSTALL_DIR} Module.symvers modules.order 
