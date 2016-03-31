
APPS := axiom_user_test
CLEANFILES = $(APPS) *.o
CFLAGS += -g -Wall -DPDEBUG -I../../axiom-docs/include/ -I../axiom_netdev_driver/
#LDFLAGS += -pthread

PWD := $(shell pwd)
#MKFILE_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
BUILDROOT := ${PWD}/../../axiom-buildroot
DESTDIR := ${BUILDROOT}/output/target
CCPREFIX := ${BUILDROOT}/output/host/usr/bin/arm-linux-
CC := ${CCPREFIX}gcc

.PHONY: all clean install

all: $(APPS)

axiom_user_test: axiom_user_test.o axiom_user_api.o

axiom_user_test.o: axiom_user_test.c axiom_user_api.h

axiom_user_api.o: axiom_user_api.c axiom_user_api.h

install: $(APPS)
	cp $(APPS) $(DESTDIR)/usr/bin/

clean:
	rm -rf $(CLEANFILES)
