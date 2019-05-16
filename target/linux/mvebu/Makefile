#
# Copyright (C) 2012-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

BOARD:=mvebu
BOARDNAME:=Marvell EBU Armada
FEATURES:=fpu usb pci pcie gpio nand squashfs ramdisk
SUBTARGETS:=cortexa9 cortexa53 cortexa72
MAINTAINER:=Imre Kaloz <kaloz@openwrt.org>

KERNEL_PATCHVER:=4.14

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += uboot-envtools kmod-gpio-button-hotplug automount autosamba v2ray luci-app-advanced-reboot

$(eval $(call BuildTarget))
