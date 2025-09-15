# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2009-2015 OpenWrt.org

include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=kirkwood
BOARDNAME:=Marvell Kirkwood
FEATURES:=usb nand squashfs ramdisk
CPU_TYPE:=xscale

KERNEL_PATCHVER:=5.4
KERNEL_TESTING_PATCHVER:=5.10

include $(INCLUDE_DIR)/target.mk

KERNELNAME:=zImage dtbs

DEFAULT_PACKAGES += uboot-envtools kmod-usb2

$(eval $(call BuildTarget))
