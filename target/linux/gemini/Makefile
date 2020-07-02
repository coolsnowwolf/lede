#
# Copyright (C) 2009-2018 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=gemini
BOARDNAME:=Cortina Systems CS351x
FEATURES:=squashfs pci rtc usb dt gpio display ext4 rootfs-part boot-part
CPU_TYPE:=fa526

KERNEL_PATCHVER:=5.4

define Target/Description
	Build firmware images for the StorLink/Cortina Gemini CS351x ARM FA526 CPU
endef

KERNELNAME:=zImage dtbs

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += \
	kmod-usb-fotg210 \
	kmod-usb-ledtrig-usbport \
	kmod-leds-gpio kmod-ledtrig-heartbeat \
	kmod-gpio-button-hotplug

$(eval $(call BuildTarget))
