
MAJOR:=0
MINOR:=0
SUBLEVEL:=0
VERSION:=${MAJOR}.${MINOR}.${SUBLEVEL}

include ../common.mk

APPS := axiom_user_test
LIBS := libaxiom_user_api.so.$(MAJOR)
SRCS_USERTEST := axiom_user_test.c
OBJS_USERTEST := $(SRCS_USERTEST:.c=.o)
DEPS_USERTEST := $(SRCS_USERTEST:.c=.d)
SRCS_USERAPI := axiom_user_api.c
OBJS_USERAPI := $(SRCS_USERAPI:.c=.o)
DEPS_USERAPI := $(SRCS_USERAPI:.c=.d)

CLEANFILES = $(APPS) $(LIBS) $(OBJS_USERTEST) $(OBJS_USERAPI) $(DEPS_USERTEST) $(DEPS_USERAPI)

CFLAGS += -Wall -fPIC $(DFLAGS) -I$(AXIOM_INCLUDE) -I$(AXIOM_DRIVER)

.PHONY: all clean install distclean

all: $(APPS) $(LIBS)

-include $(DEPS_USERTEST) $(DEPS_USERAPI)

libaxiom_user_api.so.$(MAJOR): $(OBJS_USERAPI)
	$(CC) -fPIC -shared -o $@ $?

axiom_user_test: $(OBJS_USERTEST) libaxiom_user_api.so.$(MAJOR)

PREFIX=/usr

install: all
	for DESTDIR in $(SYSROOT_DIR2); do \
		mkdir -p $${DESTDIR}$(PREFIX)/include/axiom ;\
		cp $(AXIOM_INCLUDE)/*.h $${DESTDIR}$(PREFIX)/include/axiom ;\
		mkdir -p $${DESTDIR}$(PREFIX)/lib/pkgconfig ;\
		for LIB in $(LIBS); do \
			cp $${LIB} $${DESTDIR}$(PREFIX)/lib/$${LIB}.$(MINOR).$(SUBLEVEL) ;\
			ln -sf $${LIB}.$(MINOR).$(SUBLEVEL) $${DESTDIR}$(PREFIX)/lib/$${LIB}.$(MINOR) ;\
			ln -sf $${LIB}.$(MINOR).$(SUBLEVEL) $${DESTDIR}$(PREFIX)/lib/$${LIB} ;\
		done ;\
		sed -e "s,@PREFIX@,$(PREFIX)," axiom_user_api.pc.in \
			> $${DESTDIR}$(PREFIX)/lib/pkgconfig/axiom_user_api.pc ;\
	done ;\
	for DESTDIR in $(TARGET_DIR2); do \
		mkdir -p $${DESTDIR}$(PREFIX)/bin ;\
		cp $(APPS) $${DESTDIR}$(PREFIX)/bin/ ;\
		mkdir -p $${DESTDIR}$(PREFIX)/lib ;\
		for LIB in $(LIBS); do \
			cp $${LIB} $${DESTDIR}$(PREFIX)/lib/$${LIB}.$(MINOR).$(SUBLEVEL) ;\
			ln -sf $${LIB}.$(MINOR).$(SUBLEVEL) $${DESTDIR}$(PREFIX)/lib/$${LIB}.$(MINOR) ;\
			ln -sf $${LIB}.$(MINOR).$(SUBLEVEL) $${DESTDIR}$(PREFIX)/lib/$${LIB} ;\
		done ;\
	done

clean distclean:
	rm -rf $(CLEANFILES)
