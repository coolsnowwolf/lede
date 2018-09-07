#
# Copyright (C) 2010-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=omap24xx
BOARDNAME:=TI OMAP24xx
MAINTAINER:=Michael Buesch <m@bues.ch>
FEATURES:=targz squashfs jffs2_nand usb usbgadget display gpio audio broken
CPU_TYPE:=arm1136j-s

KERNEL_PATCHVER:=4.1

KERNELNAME:=zImage dtbs

define Target/Description
	TI OMAP-24xx
endef

include $(INCLUDE_DIR)/target.mk

$(eval $(call BuildTarget))
