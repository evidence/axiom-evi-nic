AXIOM_INCLUDE := ../include

APPS := axiom_switch
HEADERS := $(AXIOM_INCLUDE)/*.h
CLEANFILES = $(APPS) *.o

DFLAGS := -g -DPDEBUG
CFLAGS += -Wall $(DFLAGS) -I$(AXIOM_INCLUDE)
LDLIBS += -lm


.PHONY: all install clean

all: $(APPS)

axiom_switch: axiom_switch.o axiom_switch_logic_default.o axiom_switch_event_loop.o


axiom_switch.o: axiom_switch.c axiom_switch_logic.h axiom_switch.h axiom_switch_event_loop.h axiom_switch_qemu.h $(HEADERS)

axiom_switch_logic_default.o: axiom_switch_logic_default.c axiom_switch_logic.h axiom_switch.h $(HEADERS)

axiom_switch_event_loop.o: axiom_switch_event_loop.h axiom_switch_event_loop.c axiom_switch.h


install: all

clean:
	rm -rf $(CLEANFILES)
