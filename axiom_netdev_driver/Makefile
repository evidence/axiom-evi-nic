
obj-m := axiom_netdev.o
axiom_netdev-objs := axiom_kernel_api.o axiom_netdev_module.o

PWD := $(shell pwd)
MKFILE_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
BUILDROOT := ${PWD}/../../axiom-evi-buildroot
KERNELDIR := ${BUILDROOT}/output/build/linux-custom
DESTDIR := ${BUILDROOT}/output/target
CCPREFIX := ${BUILDROOT}/output/host/usr/bin/arm-linux-

ccflags-y += -DPDEBUG -I${MKFILE_DIR}/../include/ -Wall

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
