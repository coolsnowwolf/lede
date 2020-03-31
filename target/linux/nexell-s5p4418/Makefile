#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=nexell-s5p4418
BOARDNAME:=NEXELL S5P4418 (ARM)
FEATURES:=fpu usb ext4 display rtc squashfs
CPU_TYPE:=cortex-a9
CPU_SUBTYPE:=neon
MAINTAINER:=FriendlyARM <support@friendlyarm.com>

KERNEL_PATCHVER:=4.19


define Target/Description
	Build firmware images for NEXELL S5P4418 based FriendlyARM S5P4418 boards routers with ARM CPU, *not* MIPS.
endef

include $(INCLUDE_DIR)/target.mk

KERNELNAME:=zImage dtbs

DEFAULT_PACKAGES += uboot-envtools mkf2fs e2fsprogs blkid

$(eval $(call BuildTarget))
