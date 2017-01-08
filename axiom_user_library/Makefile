
include ../common.mk

APPS := axiom_user_test
LIBS := libaxiom_user_api.so
SRCS_USERTEST := axiom_user_test.c
OBJS_USERTEST := $(SRCS_USERTEST:.c=.o)
DEPS_USERTEST := $(SRCS_USERTEST:.c=.d)
SRCS_USERAPI := axiom_user_api.c
OBJS_USERAPI := $(SRCS_USERAPI:.c=.o)
DEPS_USERAPI := $(SRCS_USERAPI:.c=.d)

CLEANFILES = $(APPS) $(foreach lib,$(LIBS),$(lib).*) $(OBJS_USERTEST) $(OBJS_USERAPI) $(DEPS_USERTEST) $(DEPS_USERAPI)

CFLAGS += -Wall -fPIC $(DFLAGS) -I$(AXIOM_NIC_INCLUDE) -I$(AXIOM_NIC_DRIVER)

.PHONY: all clean install distclean

all: $(APPS) $(foreach lib,$(LIBS),$(lib).$(VERSION))

-include $(DEPS_USERTEST) $(DEPS_USERAPI)

libaxiom_user_api.so.$(VERSION): $(OBJS_USERAPI)
	$(CC) -shared -Wl,-soname,libaxiom_user_api.$(MAJOR) -o $@ $^

axiom_user_test: $(OBJS_USERTEST) libaxiom_user_api.so.$(VERSION)

PREFIX:=/usr

install: all
	for DESTDIR in $(SYSROOT_DIR); do \
		mkdir -p $${DESTDIR}$(PREFIX)/include/axiom ;\
		cp $(AXIOM_NIC_INCLUDE)/*.h \
			$${DESTDIR}$(PREFIX)/include/axiom ;\
		mkdir -p $${DESTDIR}$(PREFIX)/lib/pkgconfig ;\
		for LIB in $(LIBS); do \
			cp $${LIB}.$(VERSION) $${DESTDIR}$(PREFIX)/lib/ ;\
			ln -sf $${LIB}.$(VERSION) $${DESTDIR}$(PREFIX)/lib/$${LIB}.$(MAJOR).$(MINOR) ;\
			ln -sf $${LIB}.$(MAJOR).$(MINOR) $${DESTDIR}$(PREFIX)/lib/$${LIB}.$(MAJOR) ;\
			ln -sf $${LIB}.$(MAJOR) $${DESTDIR}$(PREFIX)/lib/$${LIB} ;\
		done ;\
		sed -e "s,@PREFIX@,$(PREFIX)," \
			-e "s,@VERSION@,$(MAJOR).$(MINOR)," \
			axiom_user_api.pc.in \
			> $${DESTDIR}$(PREFIX)/lib/pkgconfig/axiom_user_api.pc ;\
	done ;\
	for DESTDIR in $(TARGET_DIR); do \
		mkdir -p $${DESTDIR}$(PREFIX)/bin ;\
		cp $(APPS) $${DESTDIR}$(PREFIX)/bin/ ;\
		mkdir -p $${DESTDIR}$(PREFIX)/lib ;\
		for LIB in $(LIBS); do \
			cp $${LIB}.$(VERSION) $${DESTDIR}$(PREFIX)/lib/ ;\
			ln -sf $${LIB}.$(VERSION) $${DESTDIR}$(PREFIX)/lib/$${LIB}.$(MAJOR).$(MINOR) ;\
			ln -sf $${LIB}.$(MAJOR).$(MINOR) $${DESTDIR}$(PREFIX)/lib/$${LIB}.$(MAJOR) ;\
		done ;\
	done

clean distclean:
	rm -rf $(CLEANFILES)
