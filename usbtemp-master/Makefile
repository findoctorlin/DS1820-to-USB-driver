LINUX_BUILD_DIR ?= /lib/modules/$(shell uname -r)/build

PWD       := $(shell pwd)

obj-m	:= usbtemp.o


all:
	$(MAKE) -C $(LINUX_BUILD_DIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(LINUX_BUILD_DIR) M=$(PWD) clean
 
