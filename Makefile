APPS_DIR := axiom_netdev_driver axiom_switch axiom_user_library
CLEAN_DIR := $(addprefix _clean_, $(APPS_DIR))
INSTALL_DIR := $(addprefix _install_, $(APPS_DIR))

PWD := $(shell pwd)

CCARCH := aarch64

ifdef CCARCH
    BUILDROOT := ${PWD}/../axiom-evi-buildroot
    DESTDIR := ${BUILDROOT}/output/target
    CCPREFIX := ${BUILDROOT}/output/host/usr/bin/$(CCARCH)-linux-
endif

DFLAGS := -g -DPDEBUG

.PHONY: all clean install $(APPS_DIR) $(CLEAN_DIR) $(INSTALL_DIR)

all: $(APPS_DIR)

libs: axiom_user_library

$(APPS_DIR):
	cd $@ && make CCARCH=$(CCARCH) CCPREFIX=$(CCPREFIX) DFLAGS="$(DFLAGS)"


install: $(INSTALL_DIR)

$(INSTALL_DIR):
	cd $(subst _install_,,$@) && make install CCARCH=$(CCARCH) CCPREFIX=$(CCPREFIX) DESTDIR=$(DESTDIR) DFLAGS="$(DFLAGS)"


clean: $(CLEAN_DIR)

$(CLEAN_DIR): _clean_%:
	cd $(subst _clean_,,$@) && make clean

