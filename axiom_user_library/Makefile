
include ../common.mk

APPS := axiom_user_test
LIBS := libaxiom_user_api.so
LIBS_INSTR := libaxiom_user_api_instr.so
SRCS_USERTEST := axiom_user_test.c
OBJS_USERTEST := $(SRCS_USERTEST:.c=.o)
DEPS_USERTEST := $(SRCS_USERTEST:.c=.d)
SRCS_USERAPI := axiom_user_api.c
OBJS_USERAPI := $(SRCS_USERAPI:.c=.o)
OBJS_USERAPI_INSTR := $(SRCS_USERAPI:.c=_instr.o)
DEPS_USERAPI := $(SRCS_USERAPI:.c=.d)

CLEANFILES = $(APPS) $(foreach lib,$(LIBS),$(lib).*) $(OBJS_USERTEST) $(OBJS_USERAPI) $(DEPS_USERTEST) $(DEPS_USERAPI)

CFLAGS += -Wall -fPIC $(DFLAGS) -I$(AXIOM_NIC_INCLUDE) -I$(AXIOM_NIC_DRIVER)

LDLIBS_INSTR := -lnanostrace
CFLAGS_INSTR := $(CFLAGS) -DAXIOM_EXTRAE_SUPPORT

.PHONY: all clean install distclean

all: $(APPS) $(foreach lib,$(LIBS),$(lib).$(VERSION)) $(foreach lib,$(LIBS_INSTR),$(lib).$(VERSION))

-include $(DEPS_USERTEST) $(DEPS_USERAPI)

libaxiom_user_api.so.$(VERSION): $(OBJS_USERAPI)
	$(CC) -shared -Wl,-soname,libaxiom_user_api.so.$(MAJOR) -o $@ $^

libaxiom_user_api_instr.so.$(VERSION): $(OBJS_USERAPI_INSTR)
	$(CC) -shared -Wl,-soname,libaxiom_user_api.so.$(MAJOR) -o $@ $^ $(LDLIBS)

%_instr.o: %.c %.d
	$(CC) $(DEPFLAGS) $(CPPFLAGS) $(CFLAGS_INSTR) -c -o $@ $<

axiom_user_test: $(OBJS_USERTEST) libaxiom_user_api.so.$(VERSION)

PREFIX:=/usr

install: all
	for DESTDIR in $(SYSROOT_INST_DIR); do \
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
		mkdir -p $${DESTDIR}$(PREFIX)/lib/instrumentation ;\
		for LIB in $(LIBS_INSTR); do \
			LIB_RENAME=$$(echo $$LIB | sed -e 's/_instr\.so/\.so/g') ;\
			cp $${LIB}.$(VERSION) $${DESTDIR}$(PREFIX)/lib/instrumentation/$${LIB_RENAME}.$(VERSION) ;\
			ln -sf $${LIB_RENAME}.$(VERSION) $${DESTDIR}$(PREFIX)/lib/instrumentation/$${LIB_RENAME}.$(MAJOR).$(MINOR) ;\
			ln -sf $${LIB_RENAME}.$(MAJOR).$(MINOR) $${DESTDIR}$(PREFIX)/lib/instrumentation/$${LIB_RENAME}.$(MAJOR) ;\
			ln -sf $${LIB_RENAME}.$(MAJOR) $${DESTDIR}$(PREFIX)/lib/instrumentation/$${LIB_RENAME} ;\
		done ;\
		sed -e "s,@PREFIX@,$(PREFIX)," \
			-e "s,@VERSION@,$(MAJOR).$(MINOR)," \
			axiom_user_api.pc.in \
			> $${DESTDIR}$(PREFIX)/lib/pkgconfig/axiom_user_api.pc ;\
	done ;\
	for DESTDIR in $(TARGET_INST_DIR); do \
		mkdir -p $${DESTDIR}$(PREFIX)/bin ;\
		cp $(APPS) $${DESTDIR}$(PREFIX)/bin/ ;\
		mkdir -p $${DESTDIR}$(PREFIX)/lib ;\
		for LIB in $(LIBS); do \
			cp $${LIB}.$(VERSION) $${DESTDIR}$(PREFIX)/lib/ ;\
			ln -sf $${LIB}.$(VERSION) $${DESTDIR}$(PREFIX)/lib/$${LIB}.$(MAJOR).$(MINOR) ;\
			ln -sf $${LIB}.$(MAJOR).$(MINOR) $${DESTDIR}$(PREFIX)/lib/$${LIB}.$(MAJOR) ;\
			ln -sf $${LIB}.$(MAJOR) $${DESTDIR}$(PREFIX)/lib/$${LIB} ;\
		done ;\
		mkdir -p $${DESTDIR}$(PREFIX)/lib/instrumentation ;\
		for LIB in $(LIBS_INSTR); do \
			LIB_RENAME=$$(echo $$LIB | sed -e 's/_instr\.so/\.so/g') ;\
			cp $${LIB}.$(VERSION) $${DESTDIR}$(PREFIX)/lib/instrumentation/$${LIB_RENAME}.$(VERSION) ;\
			ln -sf $${LIB_RENAME}.$(VERSION) $${DESTDIR}$(PREFIX)/lib/instrumentation/$${LIB_RENAME}.$(MAJOR).$(MINOR) ;\
			ln -sf $${LIB_RENAME}.$(MAJOR).$(MINOR) $${DESTDIR}$(PREFIX)/lib/instrumentation/$${LIB_RENAME}.$(MAJOR) ;\
			ln -sf $${LIB_RENAME}.$(MAJOR) $${DESTDIR}$(PREFIX)/lib/instrumentation/$${LIB_RENAME} ;\
		done ;\
	done

clean distclean:
	rm -rf $(CLEANFILES)
