# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2013-2014 OpenWrt.org

include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=imx6
BOARDNAME:=NXP i.MX 6
FEATURES:=audio display fpu gpio pcie rtc usb usbgadget squashfs targz nand ubifs boot-part rootfs-part
CPU_TYPE:=cortex-a9
CPU_SUBTYPE:=neon
MAINTAINER:=Luka Perkov <luka@openwrt.org>

KERNEL_PATCHVER:=5.4
KERNEL_TESTING_PATCHVER:=5.10

include $(INCLUDE_DIR)/target.mk

KERNELNAME:=zImage dtbs

DEFAULT_PACKAGES += uboot-envtools mkf2fs e2fsprogs blkid

$(eval $(call BuildTarget))
