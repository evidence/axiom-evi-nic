include ../common.mk

APPS := axiom_switch
SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)
DEPS := $(SRCS:.c=.d)

CLEANFILES = $(APPS) $(OBJS) $(DEPS)

CFLAGS += -Wall $(DFLAGS) -I$(AXIOM_INCLUDE)
LDLIBS += -lm

CC := gcc

.PHONY: all install clean distclean

all: $(APPS)

-include $(DEPS)

axiom_switch: $(OBJS)

PREFIX=/usr

install: all
	for DESTDIR in $(HOST_DIR); do \
		mkdir -p $${DESTDIR}$(PREFIX)/bin ;\
		cp $(APPS) $${DESTDIR}$(PREFIX)/bin ;\
	done

clean distclean:
	rm -rf $(CLEANFILES)
