
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
DEPS_USERAPI_INSTR := $(SRCS_USERAPI:.c=_instr.d)

# generated files
CLEANFILES = $(APPS) \
	$(foreach lib,$(LIBS) $(LIBS_INSTR),$(lib).*) \
	$(OBJS_USERTEST) $(OBJS_USERAPI) $(OBJS_USERAPI_INSTR) \
	$(DEPS_USERTEST) $(DEPS_USERAPI) $(DEPS_USERAPI_INSTR)

# flags
CFLAGS += -Wall -fPIC $(DFLAGS) -I$(AXIOM_NIC_INCLUDE) -I$(AXIOM_NIC_DRIVER)
LDFLAGS +=

# instrumentation flags
LDLIBS_INSTR := -lnanostrace
CFLAGS_INSTR := $(CFLAGS) -DAXIOM_EXTRAE_SUPPORT

#
# main target
#

.PHONY: all clean install distclean mrproper

all: $(APPS) $(foreach lib,$(LIBS) $(LIBS_INSTR),$(lib).$(VERSION))

clean distclean mrproper:
	rm -rf $(CLEANFILES)

-include $(DEPS_USERTEST) $(DEPS_USERAPI) $(DEPS_USERAPI_INSTR)

#
# compile/link library
#

libaxiom_user_api.so.$(VERSION): $(OBJS_USERAPI)
	$(CC) $(LFDLAGS) \
		-shared -Wl,-soname,libaxiom_user_api.so.$(MAJOR) \
		-o $@ $^

axiom_user_test: $(OBJS_USERTEST) libaxiom_user_api.so.$(VERSION)

#
# compile/link instrumentation library
#

ifeq ($(DISABLE_INSTR),1)

.PHONY: libaxiom_user_api_instr.so.$(VERSION)

libaxiom_user_api_instr.so.$(VERSION):

else

DEPFLAGS_INSTR = -MT $@ -MMD -MP -MF $*_instr.Td

%_instr.o: %.c
%_instr.o: %.c %_instr.d
	$(CC) $(DEPFLAGS_INSTR) $(CPPFLAGS) $(CFLAGS_INSTR) -c -o $@ $<
	mv -f $*_instr.Td $*_instr.d

libaxiom_user_api_instr.so.$(VERSION): $(OBJS_USERAPI_INSTR)
	$(CC) $(LDFLAGS) \
		-shared -Wl,-soname,libaxiom_user_api.so.$(MAJOR) \
		-o $@ $^ $(LDLIBS_INSTR)

endif

#
# installation
#

install:
	mkdir -p $(DESTDIR)$(PREFIX)/bin ;\
	cp $(APPS) $(DESTDIR)$(PREFIX)/bin/ ;\
	mkdir -p $(DESTDIR)$(PREFIX)/include/axiom ;\
	cp $(AXIOM_NIC_INCLUDE)/*.h $(DESTDIR)$(PREFIX)/include/axiom ;\
	mkdir -p $(DESTDIR)$(PREFIX)/lib ;\
	for LIB in $(LIBS); do \
	  cp $${LIB}.$(VERSION) $(DESTDIR)$(PREFIX)/lib/ ;\
	  ln -sf $${LIB}.$(VERSION) $(DESTDIR)$(PREFIX)/lib/$${LIB}.$(MAJOR).$(MINOR) ;\
	  ln -sf $${LIB}.$(MAJOR).$(MINOR) $(DESTDIR)$(PREFIX)/lib/$${LIB}.$(MAJOR) ;\
	  ln -sf $${LIB}.$(MAJOR) $(DESTDIR)$(PREFIX)/lib/$${LIB} ;\
	done
ifeq ($(DISABLE_INSTR),0)
	mkdir -p $(DESTDIR)$(PREFIX)/lib/instrumentation ;\
	for LIB in $(LIBS_INSTR); do \
	  LIB_RENAME=$$(echo $$LIB | sed -e 's/_instr\.so/\.so/g') ;\
	  cp $${LIB}.$(VERSION) \
		$(DESTDIR)$(PREFIX)/lib/instrumentation/$${LIB_RENAME}.$(VERSION);\
	  ln -sf $${LIB_RENAME}.$(VERSION) \
		$(DESTDIR)$(PREFIX)/lib/instrumentation/$${LIB_RENAME}.$(MAJOR).$(MINOR);\
	  ln -sf $${LIB_RENAME}.$(MAJOR).$(MINOR) \
		$(DESTDIR)$(PREFIX)/lib/instrumentation/$${LIB_RENAME}.$(MAJOR);\
	  ln -sf $${LIB_RENAME}.$(MAJOR) \
		$(DESTDIR)$(PREFIX)/lib/instrumentation/$${LIB_RENAME};\
	done
endif
	mkdir -p $(DESTDIR)$(PREFIX)/lib/pkgconfig ;\
	sed -e "s,@PREFIX@,$(PREFIX)," \
		-e "s,@VERSION@,$(MAJOR).$(MINOR)," \
		axiom_user_api.pc.in \
		> $(DESTDIR)$(PREFIX)/lib/pkgconfig/axiom_user_api.pc
