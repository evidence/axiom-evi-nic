
CCARCH := aarch64

COMMKFILE_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
#BUILDROOT := ${COMMKFILE_DIR}/../axiom-evi-buildroot
OUTPUT_DIR := ${COMMKFILE_DIR}/../output
TARGET_DIR := $(realpath ${OUTPUT_DIR}/target)
SYSROOT_DIR := $(realpath ${OUTPUT_DIR}/staging)
HOST_DIR := $(realpath ${OUTPUT_DIR}/host)

#
# various directories for THIS project
#

AXIOM_INCLUDE := $(COMMKFILE_DIR)/include
AXIOM_DRIVER :=	$(COMMKFILE_DIR)/axiom_netdev_driver
# need by nic_driver (to remove)
AXIOM_MEM_DEV_INCLUDE := $(realpath $(COMMKFILE_DIR)/../axiom-evi-allocator-drv)

ifdef CCARCH
    KERNELDIR := ${OUTPUT_DIR}/build/linux-custom
    CCPREFIX := ${HOST_DIR}/usr/bin/$(CCARCH)-linux-
ifeq ($(CCARCH), aarch64)
    CROSS_COMPILE := ARCH=arm64 CROSS_COMPILE=$(CCPREFIX)
else
    CROSS_COMPILE := ARCH=$(CCARCH) CROSS_COMPILE=$(CCPREFIX)
endif
else
    KERNELDIR := /lib/modules/$(shell uname -r)/build
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
