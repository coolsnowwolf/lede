#
# Copyright (C) 2006-2019 OpenWrt.org
# Copyright (C) 2016 LEDE project
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=mips
BOARD:=bcm63xx
BOARDNAME:=Broadcom BCM63xx
SUBTARGETS:=generic smp
FEATURES:=squashfs usb atm pci pcmcia usbgadget
KERNEL_PATCHVER:=5.4

define Target/Description
	Build firmware images for Broadcom based xDSL/routers
	currently supports BCM6338, BCM6348 and BCM6358 based devices.
	(e.g. Inventel Livebox, Siemens SE515, Neufbox 4)
endef

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += swconfig kmod-gpio-button-hotplug

$(eval $(call BuildTarget))
