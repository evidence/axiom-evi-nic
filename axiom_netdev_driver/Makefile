MKFILE_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
PWD := $(shell pwd)

include $(MKFILE_DIR)/../common.mk

obj-m := axiom_netdev.o
axiom_netdev-objs := axiom_kernel_api.o axiom_netdev_module.o axiom_kthread.o

ccflags-y += -Wall $(DFLAGS) -I${AXIOM_NIC_INCLUDE}
ccflags-y += $(AXIOM_KERNEL_CFLAGS)

ifeq ($(P)$(E),10)
CC:=$(PETALINUX)/tools/linux-i386/aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc
endif

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
