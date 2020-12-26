#
# Copyright (C) 2009-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=kirkwood
BOARDNAME:=Marvell Kirkwood
FEATURES:=usb nand squashfs ramdisk
CPU_TYPE:=xscale

KERNEL_PATCHVER:=5.4

include $(INCLUDE_DIR)/target.mk

KERNELNAME:=zImage dtbs

DEFAULT_PACKAGES += uboot-envtools kmod-usb2

$(eval $(call BuildTarget))
