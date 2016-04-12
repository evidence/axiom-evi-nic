AXIOM_INCLUDE := ../include

obj-m := axiom_netdev.o
axiom_netdev-objs := axiom_kernel_api.o axiom_netdev_module.o

PWD := $(shell pwd)
MKFILE_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

ifeq ($(CCARCH), arm)
    BUILDROOT := ${PWD}/../../axiom-evi-buildroot
    KERNELDIR := ${BUILDROOT}/output/build/linux-custom
    DESTDIR := ${BUILDROOT}/output/target
    CCPREFIX := ${BUILDROOT}/output/host/usr/bin/arm-linux-
endif

ifdef CCARCH
    CROSS_COMPILE := ARCH=$(CCARCH) CROSS_COMPILE=$(CCPREFIX)
else
    KERNELDIR := /lib/modules/$(shell uname -r)/build
    CCPREFIX :=
    CROSS_COMPILE :=
endif

DFLAGS := -g -DPDEBUG
ccflags-y += -Wall $(DFLAGS) -I${MKFILE_DIR}/${AXIOM_INCLUDE}

default:
	$(MAKE) $(CROSS_COMPILE) -C $(KERNELDIR) M=$(PWD) modules

install: default
	$(MAKE) $(CROSS_COMPILE) -C $(KERNELDIR) M=$(PWD) INSTALL_MOD_PATH=$(DESTDIR) modules_install


clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions
	rm -rf Module.symvers modules.order

depend .depend dep:
	$(CC) $(EXTRA_CFLAGS) -M *.c > .depend


ifeq (.depend,$(wildcard .depend))
	include .depend
endif
