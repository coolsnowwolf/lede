#
# Copyright (C) 2006-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=at91
MAINTAINER:=Claudio Mignanti <c.mignanti@gmail.com>
BOARDNAME:=Atmel AT91
FEATURES:=squashfs targz ext2 usb usbgadget ubifs
SUBTARGETS:=legacy sama5d3

KERNEL_PATCHVER:=4.4

include $(INCLUDE_DIR)/target.mk

KERNELNAME:=zImage dtbs

DEFAULT_PACKAGES += kmod-usb-ohci kmod-at91-udc kmod-usb-gadget-eth

$(eval $(call BuildTarget))
