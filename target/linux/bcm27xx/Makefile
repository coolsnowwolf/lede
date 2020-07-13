#
# Copyright (C) 2012-2020 OpenWrt.org
# Copyright (C) 2017 LEDE project
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=bcm27xx
BOARDNAME:=Broadcom BCM27xx
FEATURES:=ext4 audio usb usbgadget display gpio fpu squashfs rootfs-part boot-part
SUBTARGETS:=bcm2708 bcm2709 bcm2710 bcm2711

KERNEL_PATCHVER=5.4

define Target/Description
	Build firmware image for Broadcom BCM27xx SoC devices.
	Currently produces SD Card image for Raspberry Pi.
endef

include $(INCLUDE_DIR)/target.mk
DEFAULT_PACKAGES += \
	bcm27xx-gpu-fw \
	kmod-usb-hid \
	kmod-sound-core kmod-sound-arm-bcm2835 \
	kmod-fs-vfat kmod-nls-cp437 kmod-nls-iso8859-1 \
	partx-utils mkf2fs e2fsprogs

KERNELNAME:=Image dtbs

$(eval $(call BuildTarget))
