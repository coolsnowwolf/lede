# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (c) 2023 Phytium Technology Co., Ltd.
# Copyright (C) 2023-2024 Ailick <277498654@qq.com>

include $(TOPDIR)/rules.mk

BOARD:=phytium
BOARDNAME:=Phytium
FEATURES:=fpu pci pcie rtc usb boot-part rootfs-part
FEATURES+=cpiogz ext4 ramdisk squashfs targz vmdk usb gpio fpu pci pcie jffs2
SUBTARGETS:=armv8 e2000 phytiumpi

KERNEL_PATCHVER:=5.10

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += mkf2fs e2fsprogs
# blkid used for resolving PARTUUID
# in sysupgrade. vfat required for
# mounting ESP partition
DEFAULT_PACKAGES += blkid kmod-fs-vfat kmod-usb2 kmod-usb3 kmod-hwmon-phytium

$(eval $(call BuildTarget))
