MKFILE_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
PWD := $(shell pwd)

include $(MKFILE_DIR)/../common.mk

obj-m := axiom_netdev.o
axiom_netdev-objs := axiom_netdev_common.o axiom_kthread.o axiom_netdev_sysfs.o
ifeq ($(MODE),aarch64)
axiom_netdev-objs += axiom_netdev_arm64.o axiom_kernel_api_arm64.o
else
axiom_netdev-objs += axiom_netdev_x86.o
endif

CFLAGS:=

ccflags-y += -Wall $(DFLAGS) -I${AXIOM_NIC_INCLUDE}
ccflags-y += $(AXIOM_KERNEL_CFLAGS)

default::
	$(MAKE) $(CROSS_COMPILE) -C $(KERNEL_DIR) M=$(MKFILE_DIR) \
	  modules

install: default
	$(MAKE) $(CROSS_COMPILE) -C $(KERNEL_DIR) M=$(MKFILE_DIR) \
	  INSTALL_MOD_PATH=$(DESTDIR) \
	  modules_install

clean distclean mrproper:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions
	rm -rf Module.symvers modules.order

depend .depend dep:
	$(CC) $(EXTRA_CFLAGS) -M *.c > .depend

ifeq (.depend,$(wildcard .depend))
	include .depend
endif
