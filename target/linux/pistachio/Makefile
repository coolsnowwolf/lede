#
# Copyright (C) 2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=mipsel
BOARD:=pistachio
BOARDNAME:=MIPS pistachio
FEATURES:=fpu usb usbgadget squashfs targz nand
CPU_TYPE:=24kc
CPU_SUBTYPE:=24kf

KERNEL_PATCHVER:=4.14

include $(INCLUDE_DIR)/target.mk

KERNELNAME:=vmlinux dtbs

DEFAULT_PACKAGES += \
	kmod-gpio-button-hotplug \
	uboot-envtools

$(eval $(call BuildTarget))
