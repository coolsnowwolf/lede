#
# Copyright (C) 2009 OpenWrt.org
#

SUBTARGET:=mt7621
BOARDNAME:=MT7621 based boards
FEATURES+=nand ramdisk rtc usb minor
CPU_TYPE:=24kc

KERNEL_PATCHVER:=5.4
DEFAULT_PACKAGES += swconfig

define Target/Description
	Build firmware images for Ralink MT7621 based boards.
endef

