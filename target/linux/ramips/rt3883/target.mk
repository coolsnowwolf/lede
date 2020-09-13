#
# Copyright (C) 2011 OpenWrt.org
#

SUBTARGET:=rt3883
BOARDNAME:=RT3662/RT3883 based boards
FEATURES+=usb pci small_flash
CPU_TYPE:=74kc

DEFAULT_PACKAGES += kmod-rt2800-pci kmod-rt2800-soc wpad-mini swconfig

define Target/Description
	Build firmware images for Ralink RT3662/RT3883 based boards.
endef

