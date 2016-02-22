
obj-m := axiom_netdev.o

BUILDROOT := /home/stefano/git/buildroot-xilinx
KERNELDIR := ${BUILDROOT}/output/build/linux-custom
DESTDIR := ${BUILDROOT}/output/target
CCPREFIX := ${BUILDROOT}/output/host/usr/bin/arm-linux-
PWD := $(shell pwd)

default:
	$(MAKE) ARCH=arm CROSS_COMPILE=${CCPREFIX} -C $(KERNELDIR) M=$(PWD) modules

install: default
	$(MAKE) ARCH=arm CROSS_COMPILE=${CCPREFIX} -C $(KERNELDIR) M=$(PWD) INSTALL_MOD_PATH=$(DESTDIR) modules_install

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions
	rm -rf Module.symvers modules.order

depend .depend dep:
	$(CC) $(EXTRA_CFLAGS) -M *.c > .depend


ifeq (.depend,$(wildcard .depend))
	include .depend
endif
