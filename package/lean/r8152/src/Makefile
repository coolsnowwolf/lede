#
#
#

CONFIG_CTAP_SHORT = ON

ifneq ($(KERNELRELEASE),)
	obj-m	 := r8152.o
#	EXTRA_CFLAGS += -DRTL8152_S5_WOL
#	EXTRA_CFLAGS += -DRTL8152_DEBUG
	ifneq (,$(filter OFF off, $(CONFIG_CTAP_SHORT)))
		EXTRA_CFLAGS += -DCONFIG_CTAP_SHORT_OFF
	endif
else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD :=$(shell pwd)
	TARGET_PATH := kernel/drivers/net/usb
	INBOXDRIVER := $(shell find $(subst build,$(TARGET_PATH),$(KERNELDIR)) -name r8152.ko.* -type f)
	RULEFILE = 50-usb-realtek-net.rules
	RULEDIR = /etc/udev/rules.d/

.PHONY: modules
modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

.PHONY: all
all: clean modules install

.PHONY: clean
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

.PHONY: install
install:
ifneq ($(shell lsmod | grep r8152),)
	rmmod r8152
endif
ifneq ($(INBOXDRIVER),)
	rm -f $(INBOXDRIVER)
endif
	$(MAKE) -C $(KERNELDIR) M=$(PWD) INSTALL_MOD_DIR=$(TARGET_PATH) modules_install
	modprobe r8152

.PHONY: install_rules
install_rules:
	install --group=root --owner=root --mode=0644 $(RULEFILE) $(RULEDIR)

endif

