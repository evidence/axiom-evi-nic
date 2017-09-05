
#
# default architecture
#

ifeq ($(FS),x86)
undefine CCARCH
else
CCARCH := aarch64
endif

#
# version numbers
#

VERSION := $(shell git describe --tags --dirty | sed -re 's/^axiom-v([0-9.]*).*/\1/')
MAJOR := $(shell echo $(VERSION) | sed -e 's/\..*//')
MINOR := $(shell echo $(VERSION) | sed -re 's/[0-9]*\.([0-9]*).*/\1/')
SUBLEVEL := $(shell echo $(VERSION) | sed -r -e 's/([0-9]*\.){2}([0-9]*).*/\2/' -e 's/[0-9]*\.[0-9]*/0/')
VERSION := $(MAJOR).$(MINOR).$(SUBLEVEL)

#
# variuos output directories
#

COMMKFILE_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
OUTPUT_DIR := ${COMMKFILE_DIR}/../output
ifeq ($(FS),seco)
TARGET_DIR := $(realpath ${ROOTFS})
SYSROOT_DIR := $(realpath ${ROOTFS})
HOST_DIR := $(realpath ${LINARO}/host)
else
ifeq ($(FS),x86)
TARGET_DIR := /
SYSROOT_DIR := /
HOST_DIR := /
else
TARGET_DIR := $(realpath ${OUTPUT_DIR}/target)
SYSROOT_DIR := $(realpath ${OUTPUT_DIR}/staging)
HOST_DIR := $(realpath ${OUTPUT_DIR}/host)
endif
endif

SYSROOT_INST_DIR ?= $(SYSROOT_DIR)
TARGET_INST_DIR ?= $(TARGET_DIR)

# fakeroot

FAKEROOT :=
ifeq ($(FS),seco)
ifndef FAKEROOTKEY
FAKEROOT := fakeroot -i $(ROOTFS).faked -s $(ROOTFS).faked
endif
endif

#
# internal directory structure & tools
#

AXIOM_NIC_INCLUDE := $(COMMKFILE_DIR)/include
AXIOM_NIC_DRIVER := $(COMMKFILE_DIR)/axiom_netdev_driver
# to find the header files of the axiom-evi-allocator-drv kernel module
AXIOM_KERNEL_CFLAGS := -I$(realpath $(SYSROOT_DIR)/usr/include/linux)

ifdef CCARCH
# defined CCARCH

ifeq ($(KERN),seco)
    CCPREFIX := ${HOST_DIR}/usr/bin/$(CCARCH)-linux-gnu-
else
    CCPREFIX := ${HOST_DIR}/usr/bin/$(CCARCH)-linux-
endif

ifeq ($(KERN),seco)
    KERNELDIR := $(AXIOMBSP)/build/linux/kernel/link-to-kernel-build
    CROSS_COMPILE := ARCH=arm64 CROSS_COMPILE=$(PETALINUX)/tools/linux-i386/aarch64-linux-gnu/bin/aarch64-linux-gnu-
    AXIOM_KERNEL_CFLAGS := -I$(realpath $(SYSROOT_DIR)/usr/include/linux)

else
    KERNELDIR := ${OUTPUT_DIR}/build/linux-custom
ifeq ($(CCARCH), aarch64)
    CROSS_COMPILE := ARCH=arm64 CROSS_COMPILE=$(CCPREFIX)
else
    CROSS_COMPILE := ARCH=$(CCARCH) CROSS_COMPILE=$(CCPREFIX)
endif
endif

else
# undefined CCARCH

ifeq ($(FS),x86)
    KERNELVER :=  $(shell uname -r)
else
    KERNELVER :=  $(shell uname -r)
endif

    KERNELDIR := /lib/modules/$(KERNELVER)/build
    CCPREFIX :=
    CROSS_COMPILE :=

endif

CC := ${CCPREFIX}gcc
AR := ${CCPREFIX}ar
RANLIB := ${CCPREFIX}ranlib

#DFLAGS := -g -DPDEBUG
DFLAGS := -g

#
# source file dependencies management
#

DEPFLAGS = -MT $@ -MMD -MP -MF $*.Td

%.o: %.c
%.o: %.c %.d
	$(CC) $(DEPFLAGS) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<
	mv -f $*.Td $*.d

%.d: ;
.PRECIOUS: %.d
