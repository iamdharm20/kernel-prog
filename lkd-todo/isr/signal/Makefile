obj-m += send_signal.o
obj-m += signal_handling.o

INSTALLED_PATH=/lib/modules/$(shell uname -r)/extra/
clean-files := $(INSTALLED_PATH)/*.ko

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
modules_install:
	make -C /lib/modules/$(shell uname -r)/build INSTALL_MOD_DIR=extra M=$(PWD) modules_install
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
