#
# Copyright (C) 2008-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=orion
BOARDNAME:=Marvell Orion
FEATURES:=broken rtc
SUBTARGETS:=generic harddisk
MAINTAINER:=Imre Kaloz <kaloz@openwrt.org>

KERNEL_PATCHVER:=4.9

include $(INCLUDE_DIR)/target.mk

KERNELNAME:=zImage

DEFAULT_PACKAGES += kmod-ath9k wpad-mini kmod-rtc-isl1208 uboot-envtools

$(eval $(call BuildTarget))
