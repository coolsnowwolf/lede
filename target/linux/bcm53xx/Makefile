#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=bcm53xx
BOARDNAME:=Broadcom BCM47xx/53xx (ARM)
FEATURES:=squashfs nand usb pci pcie gpio
CPU_TYPE:=cortex-a9
SUBTARGETS:=generic

KERNEL_PATCHVER:=4.19
KERNEL_TESTING_PATCHVER:=5.4

define Target/Description
	Build firmware images for Broadcom based BCM47xx/53xx routers with ARM CPU, *not* MIPS.
endef

include $(INCLUDE_DIR)/target.mk

KERNELNAME:=zImage dtbs

DEFAULT_PACKAGES += swconfig nvram \
	osafeloader oseama otrx \
	kmod-gpio-button-hotplug \
	kmod-leds-gpio kmod-ledtrig-default-on kmod-ledtrig-timer

$(eval $(call BuildTarget))
