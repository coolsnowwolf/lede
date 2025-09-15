# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2017 Yousong Zhou <yszhou4tech@gmail.com>

include $(TOPDIR)/rules.mk

BOARD:=armsr
BOARDNAME:=Arm SystemReady (EFI) compliant
FEATURES:=fpu pci pcie rtc usb boot-part rootfs-part
FEATURES+=cpiogz ext4 ramdisk squashfs targz vmdk

KERNEL_PATCHVER:=6.6
KERNEL_TESTING_PATCHVER:=6.12
include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += mkf2fs e2fsprogs
# blkid used for resolving PARTUUID
# in sysupgrade. vfat required for
# mounting ESP partition
DEFAULT_PACKAGES += blkid kmod-fs-vfat

$(eval $(call BuildTarget))
