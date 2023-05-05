# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2008-2011 OpenWrt.org

include $(TOPDIR)/rules.mk

ARCH:=mipsel
BOARD:=ramips
BOARDNAME:=MediaTek Ralink MIPS
SUBTARGETS:=mt7620 mt7621 mt76x8 rt288x rt305x rt3883
FEATURES:=squashfs gpio boot-part rootfs-part

KERNEL_PATCHVER:=5.10
KERNEL_TESTING_PATCHVER:=5.4

define Target/Description
	Build firmware images for Ralink RT288x/RT3xxx based boards.
endef

include $(INCLUDE_DIR)/target.mk
DEFAULT_PACKAGES += kmod-leds-gpio kmod-gpio-button-hotplug

$(eval $(call BuildTarget))
