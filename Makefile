
export KERN
export FS
export DISABLE_INSTR

ifeq ($(FS),x86)
APPS_DIR := axiom_user_library axiom_netdev_driver
else
APPS_DIR := axiom_user_library axiom_netdev_driver axiom_switch
endif

.PHONY: all clean distclean install $(APPS_DIR)

all: $(APPS_DIR)

libs: axiom_user_library
	$(MAKE) -C axiom_user_library install

install clean distclean:
	for DIR in $(APPS_DIR); do { $(MAKE) -C $$DIR $@ || exit 1; }; done

$(APPS_DIR):
	$(MAKE) -C $@
