# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2017 Yousong Zhou <yszhou4tech@gmail.com>

include $(TOPDIR)/rules.mk

BOARD:=armvirt
BOARDNAME:=QEMU ARM Virtual Machine
FEATURES:=fpu pci pcie rtc usb boot-part rootfs-part
FEATURES+=cpiogz ext4 ramdisk squashfs targz vmdk

KERNEL_PATCHVER:=5.15

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += mkf2fs e2fsprogs

$(eval $(call BuildTarget))
