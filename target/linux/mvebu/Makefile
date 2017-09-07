#
# Copyright (C) 2012-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=mvebu
BOARDNAME:=Marvell Armada 37x/38x/XP
FEATURES:=fpu usb pci pcie gpio nand squashfs
CPU_TYPE:=cortex-a9
CPU_SUBTYPE:=vfpv3
MAINTAINER:=Imre Kaloz <kaloz@openwrt.org>

KERNEL_PATCHVER:=4.4

include $(INCLUDE_DIR)/target.mk

KERNELNAME:=zImage dtbs

DEFAULT_PACKAGES += uboot-envtools kmod-gpio-button-hotplug

$(eval $(call BuildTarget))
