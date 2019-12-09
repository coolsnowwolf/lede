#
# Copyright (C) 2008-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=mipsel
BOARD:=ramips
BOARDNAME:=MediaTek Ralink MIPS
SUBTARGETS:=mt7620 mt7621 mt76x8 rt288x rt305x rt3883
FEATURES:=squashfs gpio
MAINTAINER:=John Crispin <john@phrozen.org>

KERNEL_PATCHVER:=4.14

define Target/Description
	Build firmware images for Ralink RT288x/RT3xxx based boards.
endef

include $(INCLUDE_DIR)/target.mk
DEFAULT_PACKAGES += \
	kmod-leds-gpio kmod-gpio-button-hotplug swconfig

$(eval $(call BuildTarget))
