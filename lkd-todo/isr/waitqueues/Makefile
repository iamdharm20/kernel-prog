INSTALL_DIR=modules
ifneq (${KERNELRELEASE},)
	obj-m := wait_queue.o wait_queue4.o scd_fops.o irq.o timer.o timer1.o poll.o
module-objs := wait_queue.o wait_queue4.o scd_fops.o irq.o timer.o timer1.opoll.o

else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)
default:
	$(MAKE)  -C $(KERNELDIR) M=$(PWD) modules
	@rm -rf ${INSTALL_DIR}
	@mkdir ${INSTALL_DIR}
	@mv -f *.o *.ko *.mod.c .*.cmd ${INSTALL_DIR}
clean:
	rm -rf ${INSTALL_DIR} modules.order Module.symvers
endif
