#
# Copyright (C) 2009 OpenWrt.org
#

SUBTARGET:=mt76x8
BOARDNAME:=MT76x8 based boards
FEATURES+=usb ramdisk
CPU_TYPE:=24kc

KERNEL_PATCHVER:=5.10

DEFAULT_PACKAGES += kmod-mt7603 wpad-openssl swconfig

define Target/Description
	Build firmware images for Ralink MT76x8 based boards.
endef

