AXIOM_INCLUDE := ../include

APPS := axiom_switch
CLEANFILES = $(APPS) *.o
DFLAGS := -g -DPDEBUG
CFLAGS += -Wall $(DFLAGS) -I$(AXIOM_INCLUDE)


.PHONY: all clean

all: $(APPS)

axiom_switch: axiom_switch.o axiom_switch_logic_default.o axiom_switch_event_loop.o

axiom_switch.o: axiom_switch.c axiom_switch_logic.h axiom_switch.h axiom_switch_event_loop.h axiom_switch_qemu.h

axiom_switch_logic_default.o: axiom_switch_logic_default.c axiom_switch_logic.h axiom_switch.h

axiom_switch_event_loop.o: axiom_switch_event_loop.h axiom_switch_event_loop.c axiom_switch.h


clean:
	rm -rf $(CLEANFILES)
