#
# Copyright (C) 2015 OpenWrt.org
#

SUBTARGET:=bcm2708
BOARDNAME:=BCM2708 based boards
CPU_TYPE:=arm1176jzf-s
CPU_SUBTYPE:=vfp

define Target/Description
	Build firmware image for Broadcom BCM2708 SoC devices.
endef
