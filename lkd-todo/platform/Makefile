# If called directly from the command line, invoke the kernel build system.
#
#ifeq ($(KERNELRELEASE),)
#
#	KERNEL_SOURCE := ../../Kernel/linux-3.12.9
#	PWD := $(shell pwd)
#default:
#	$(MAKE) -C $(KERNEL_SOURCE) SUBDIRS=$(PWD) modules CROSS_COMPILE=arm-linux-gnueabihf- ARCH=arm
#
#clean:
#	$(MAKE) -C $(KERNEL_SOURCE) SUBDIRS=$(PWD) clean
#
# Otherwise KERNELRELEASE is defined; we've been invoked from the
# kernel build system and can use its language.
#else
#
#	obj-m := platform_driver.o platform_device.o platform_leds.o \
#		gpio_device.o platform_gpio_driver.o gpio.o
#
#endif

INSTALL_DIR=modules
ifneq (${KERNELRELEASE},)
	obj-m := platform_driver.o platform_device.o platform_leds.o \
		gpio_device.o platform_gpio_driver.o gpio.o

#module-objs := netdev_test.o

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
