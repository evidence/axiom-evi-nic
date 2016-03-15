
APPS := axiom_switch
CLEANFILES = $(APPS) *.o
CFLAGS += -g -I../../axiom-docs/axiom_nic/
#LDFLAGS += -pthread

PWD := $(shell pwd)
DESTDIR := $(PWD)

.PHONY: all clean install

all: $(APPS)

axiom_switch: axiom_switch.o swithc_msg_management.o

axiom_switch.o: axiom_switch.c swithc_msg_management.h

swithc_msg_management.o: swithc_msg_management.c swithc_msg_management.h

install: $(APPS)
	cp $(APPS) $(DESTDIR)/usr/bin/

clean:
	rm -rf $(CLEANFILES)
