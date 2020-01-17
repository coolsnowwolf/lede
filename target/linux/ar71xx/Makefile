#
# Copyright (C) 2008-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=mips
BOARD:=ar71xx
BOARDNAME:=Atheros AR7xxx/AR9xxx
FEATURES:=usbgadget
CPU_TYPE:=24kc
SUBTARGETS:=generic tiny nand mikrotik

KERNEL_PATCHVER:=4.14

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += \
	kmod-gpio-button-hotplug swconfig \
	kmod-ath9k wpad-openssl uboot-envtools

$(eval $(call BuildTarget))
