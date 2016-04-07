AXIOM_INCLUDE := ../include
AXIOM_DRIVER :=	../axiom_netdev_driver

APPS := axiom_user_test
HEADERS := $(AXIOM_INCLUDE)/*.h
LIBS := axiom_user_api.a
CLEANFILES = $(APPS) $(LIBS) *.o

PWD := $(shell pwd)
#MKFILE_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
BUILDROOT := ${PWD}/../../axiom-evi-buildroot
DESTDIR := ${BUILDROOT}/output/target
CCPREFIX := ${BUILDROOT}/output/host/usr/bin/arm-linux-
CC := ${CCPREFIX}gcc
AR := ${CCPREFIX}ar
RANLIB := ${CCPREFIX}ranlib

DFLAGS := -g -DPDEBUG
CFLAGS += -Wall $(DFLAGS) -I$(AXIOM_INCLUDE) -I$(AXIOM_DRIVER)

.PHONY: all clean install

all: $(APPS) $(LIBS)

axiom_user_api.a: axiom_user_api.o
	$(AR) rcs $@ $?
	$(RANLIB) $@

axiom_user_api.o: axiom_user_api.c $(HEADERS) $(AXIOM_DRIVER)/axiom_netdev_user.h


axiom_user_test: axiom_user_test.o axiom_user_api.a

axiom_user_test.o: axiom_user_test.c $(HEADERS)


install: all
	cp $(APPS) $(DESTDIR)/usr/bin/

clean:
	rm -rf $(CLEANFILES)
