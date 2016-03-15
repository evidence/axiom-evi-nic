
APPS := axiom_switch
CLEANFILES = $(APPS) *.o
CFLAGS += -g -I../../axiom-docs/axiom_nic/
#LDFLAGS += -pthread

PWD := $(shell pwd)
DESTDIR := $(PWD)

.PHONY: all clean install

all: $(APPS)

axiom_switch: axiom_switch.o axiom_switch_logic.o

axiom_switch.o: axiom_switch.c axiom_switch_logic.h

axiom_switch_logic.o: axiom_switch_logic.c axiom_switch_logic.h

install: $(APPS)
	cp $(APPS) $(DESTDIR)/usr/bin/

clean:
	rm -rf $(CLEANFILES)
