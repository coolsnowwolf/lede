#
# Copyright (C) 2006-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=mips
BOARD:=ath25
BOARDNAME:=Atheros AR231x/AR5312
FEATURES:=squashfs low_mem small_flash

KERNEL_PATCHVER:=5.4

define Target/Description
	Build firmware images for Atheros SoC boards
endef

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += wpad-basic-wolfssl kmod-ath5k swconfig kmod-gpio-button-hotplug

$(eval $(call BuildTarget))
