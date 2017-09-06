#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=oxnas
BOARDNAME:=PLXTECH/Oxford NAS782x/OX82x
DEVICE_TYPE:=nas
FEATURES:=gpio nand pcie usb ramdisk rtc squashfs ubifs
CPU_TYPE:=mpcore

MAINTAINER:=Daniel Golle <daniel@makrotopia.org>

KERNEL_PATCHVER:=4.4

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += \
	kmod-ata-core kmod-ata-oxnas-sata kmod-button-hotplug \
	kmod-input-gpio-keys-polled kmod-usb-ledtrig-usbport \
	kmod-ledtrig-timer kmod-leds-gpio kmod-usb2-oxnas \
	kmod-usb-storage uboot-envtools uboot-oxnas-ox820

KERNELNAME:=zImage dtbs

$(eval $(call BuildTarget))
