# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2017 OpenWrt.org

include $(TOPDIR)/rules.mk

ARCH:=mipsel
BOARD:=pistachio
BOARDNAME:=MIPS pistachio
FEATURES:=fpu usb usbgadget squashfs targz nand
CPU_TYPE:=24kc
CPU_SUBTYPE:=24kf
SUBTARGETS:=generic

KERNEL_PATCHVER:=5.4
KERNEL_TESTING_PATCHVER:=6.18

include $(INCLUDE_DIR)/target.mk

KERNELNAME:=vmlinux dtbs

DEFAULT_PACKAGES += \
	kmod-gpio-button-hotplug \
	uboot-envtools

$(eval $(call BuildTarget))
