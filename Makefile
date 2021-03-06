
include configure.mk

APPS_DIR := axiom_user_library axiom_netdev_driver

.PHONY: all clean distclean install $(APPS_DIR) configure mrproper

.DEFAULT_GOAL := all
all: $(APPS_DIR)

libs: axiom_user_library

libs-install: libs
	$(MAKE) -C axiom_user_library install

libs-clean libs-distclean libs-mrproper:
	$(MAKE) -C axiom_user_library $(subst libs-,,$@)

install clean distclean mrproper:
	for DIR in $(APPS_DIR); do { $(MAKE) -C $$DIR $@ || exit 1; }; done
	if [ "$@" = "distclean" -o "$@" = "mrproper" ]; then \
		$(MAKE) NCONF=1 configure-clean ;\
	fi

$(APPS_DIR):
	$(MAKE) -C $@

configure: configure-save
