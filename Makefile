
include configure.mk

APPS_DIR := axiom_user_library axiom_netdev_driver

.PHONY: all clean distclean install $(APPS_DIR) configure

.DEFAULT_GOAL := all
all: $(APPS_DIR)

libs: axiom_user_library

libs-install: libs
	$(MAKE) -C axiom_user_library install

install clean distclean mrproper:
	for DIR in $(APPS_DIR); do { $(MAKE) -C $$DIR $@ || exit 1; }; done

$(APPS_DIR):
	$(MAKE) -C $@

configure: configure-save
