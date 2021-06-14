# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2006-2013 OpenWrt.org

include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=at91
MAINTAINER:=Claudio Mignanti <c.mignanti@gmail.com> \
   Sandeep Sheriker <sandeep.sheriker@gmail.com>
BOARDNAME:=Microchip (Atmel AT91)
FEATURES:=ext4 squashfs targz usb usbgadget ubifs
SUBTARGETS:=sama5 sam9x

KERNEL_PATCHVER:=5.4

include $(INCLUDE_DIR)/target.mk

KERNELNAME:=zImage dtbs

DEFAULT_PACKAGES += kmod-usb-ohci kmod-at91-udc kmod-usb-gadget-eth

$(eval $(call BuildTarget))
