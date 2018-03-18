#
# Copyright (C) 2017 Yousong Zhou <yszhou4tech@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

BOARD:=armvirt
BOARDNAME:=QEMU ARM Virtual Machine
FEATURES:=fpu pci rtc usb
FEATURES+=cpiogz ext4 ramdisk squashfs targz
MAINTAINER:=Florian Fainelli <f.fainelli@gmail.com>, \
	Yousong Zhou <yszhou4tech@gmail.com>

KERNEL_PATCHVER:=4.14

include $(INCLUDE_DIR)/target.mk

$(eval $(call BuildTarget))
