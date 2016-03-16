
APPS := axiom_switch
CLEANFILES = $(APPS) *.o
CFLAGS += -g -I../../axiom-docs/include/ -Wall
#LDFLAGS += -pthread

PWD := $(shell pwd)
DESTDIR := $(PWD)

.PHONY: all clean install

all: $(APPS)

axiom_switch: axiom_switch.o axiom_switch_logic_default.o

axiom_switch.o: axiom_switch.c axiom_switch_logic.h axiom_switch_configuration.h

axiom_switch_logic_default.o: axiom_switch_logic_default.c axiom_switch_logic.h axiom_switch_configuration.h

install: $(APPS)
	cp $(APPS) $(DESTDIR)/usr/bin/

clean:
	rm -rf $(CLEANFILES)
