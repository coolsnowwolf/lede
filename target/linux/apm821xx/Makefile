# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk

ARCH:=powerpc
BOARD:=apm821xx
BOARDNAME:=AppliedMicro APM821xx
CPU_TYPE:=464fp
FEATURES:=fpu dt gpio
MAINTAINER:=Chris Blake <chrisrblake93@gmail.com>, \
	    Christian Lamparter <chunkeey@gmail.com>
SUBTARGETS:=nand sata

KERNEL_PATCHVER:=5.4
KERNEL_TESTING_PATCHVER:=5.10

define Target/Description
	Build images for AppliedMicro APM821xx based boards.
endef

include $(INCLUDE_DIR)/target.mk

KERNELNAME:=uImage

DEFAULT_PACKAGES += \
	kmod-leds-gpio kmod-i2c-core kmod-gpio-button-hotplug

$(eval $(call BuildTarget))
