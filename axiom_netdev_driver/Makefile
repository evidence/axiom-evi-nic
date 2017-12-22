MKFILE_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
PWD := $(shell pwd)

include $(MKFILE_DIR)/../common.mk

obj-m := axiom_netdev.o
axiom_netdev-objs := axiom_netdev_common.o axiom_kthread.o axiom_netdev_sysfs.o
ifeq ($(ARCH), arm64)
ifeq ($(KERN),seco)
axiom_netdev-objs += axiom_netdev_arm64.o axiom_kernel_api_arm64.o
else
axiom_netdev-objs += axiom_netdev_qemuarm64.o axiom_kernel_api_qemuarm64.o
endif
else
axiom_netdev-objs += axiom_netdev_x86.o
endif


ccflags-y += -Wall $(DFLAGS) -I${AXIOM_NIC_INCLUDE}
ccflags-y += $(AXIOM_KERNEL_CFLAGS)

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
