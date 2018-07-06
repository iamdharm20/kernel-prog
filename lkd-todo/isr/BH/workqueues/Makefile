INSTALL_DIR=modules
ifneq (${KERNELRELEASE},)
	obj-m := workQueue.o workQueueDelayed.o mutex.o job_list.o zombiehunter.o wq_sample1.o int_wq_sample.o
module-objs := workQueue.o workQueueDelayed.o mutex.o job_list.o zombiehunter.o wq_sample1.o int_wq_sample.o

else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)
default:
	$(MAKE)  -C $(KERNELDIR) M=$(PWD) modules
	@rm -rf ${INSTALL_DIR}
	@mkdir ${INSTALL_DIR}
	@mv -f *.o *.ko *.mod.c .*.cmd ${INSTALL_DIR}
clean:
	rm -rf ${INSTALL_DIR} Module.symvers modules.order
endif
