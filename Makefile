
include common.mk

APPS_DIR := axiom_user_library axiom_netdev_driver axiom_switch

.PHONY: all clean distclean install $(APPS_DIR)

all: $(APPS_DIR)

libs: axiom_user_library
	make -C axiom_user_library install

install clean distclean:
	for DIR in $(APPS_DIR); do make -C $$DIR $@; done

$(APPS_DIR):
	make -C $@
