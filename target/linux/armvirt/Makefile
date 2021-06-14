# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2017 Yousong Zhou <yszhou4tech@gmail.com>

include $(TOPDIR)/rules.mk

BOARD:=armvirt
BOARDNAME:=QEMU ARM Virtual Machine
FEATURES:=fpu pci rtc usb
FEATURES+=cpiogz ext4 ramdisk squashfs targz
MAINTAINER:=Florian Fainelli <f.fainelli@gmail.com>, \
	Yousong Zhou <yszhou4tech@gmail.com>

KERNEL_PATCHVER:=5.4
KERNEL_TESTING_PATCHVER:=5.10

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += mkf2fs e2fsprogs

$(eval $(call BuildTarget))
