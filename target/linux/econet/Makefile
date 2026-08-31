# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2009-2025 OpenWrt.org

include $(TOPDIR)/rules.mk

BOARD:=econet
BOARDNAME:=EcoNet EN75xx MIPS
FEATURES:=dt source-only squashfs nand usb
SUBTARGETS:=en751221 en751627 en7528

KERNEL_PATCHVER:=6.18
KERNEL_TESTING_PATCHVER:=6.18

define Target/Description
	Build firmware image for EcoNet EN75xx MIPS based boards.
endef

# include the profiles
include $(INCLUDE_DIR)/target.mk

# nand-utils is used by base-files/sbin/en75_chboot
DEFAULT_PACKAGES += nand-utils \
	kmod-econet-eth \
	kmod-leds-gpio \
	gpio-button-hotplug

$(eval $(call BuildTarget))
