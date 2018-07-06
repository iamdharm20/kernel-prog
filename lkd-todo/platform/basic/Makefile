obj−m += dummy_platform_device.o
obj−m += dummy_platform_driver.o
obj-m += sysfs.o
obj-m += platform_device_driver.o
obj-m += sysfs.o
obj-m += platform_driver_sysfs.o
obj-m += platform_ioremap.o
obj-m += platform_spi_driver.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
