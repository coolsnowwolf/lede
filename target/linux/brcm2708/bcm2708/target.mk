#
# Copyright (C) 2015-2019 OpenWrt.org
#

SUBTARGET:=bcm2708
BOARDNAME:=BCM2708 boards (32 bit)
CPU_TYPE:=arm1176jzf-s
CPU_SUBTYPE:=vfp

define Target/Description
	Build firmware image for BCM2708 devices.
	This firmware features a 32 bit kernel.
endef
