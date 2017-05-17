MKFILE_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
PWD := $(shell pwd)

include $(MKFILE_DIR)/../common.mk

obj-m := axiom_netdev.o
axiom_netdev-objs := axiom_kernel_api.o axiom_netdev_module.o axiom_kthread.o

ccflags-y += -Wall $(DFLAGS) -I${AXIOM_NIC_INCLUDE}
ccflags-y += $(AXIOM_KERNEL_CFLAGS)

CROSS_COMPILE=ARCH=arm64 CROSS_COMPILE=/home/massy/petalinux/tools/linux-i386/aarch64-linux-gnu/bin/aarch64-linux-gnu-
KERNELDIR=/home/massy/bsp/AXIOM-ZU9EG-2016.3/build/linux/kernel/link-to-kernel-build
AXIOM_KERNEL_CFLAGS=-I$(realpath $(SYSROOT_DIR)/usr/include/linux)
#AXIOM_KERNEL_CFLAGS=
CC=/home/massy/petalinux/tools/linux-i386/aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc

default::
	$(MAKE) $(CROSS_COMPILE) -C $(KERNELDIR) M=$(PWD) modules

install: default
	$(FAKEROOT) $(MAKE) $(CROSS_COMPILE) -C $(KERNELDIR) M=$(PWD) INSTALL_MOD_PATH=$(TARGET_INST_DIR) modules_install

clean distclean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions
	rm -rf Module.symvers modules.order

depend .depend dep:
	$(CC) $(EXTRA_CFLAGS) -M *.c > .depend

ifeq (.depend,$(wildcard .depend))
	include .depend
endif
