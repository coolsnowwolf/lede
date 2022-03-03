#
# Copyright (C) 2015-2019 OpenWrt.org
# Copyright (C) 2017 LEDE project
#

SUBTARGET:=bcm2709
BOARDNAME:=BCM2709/BCM2710/BCM2711 boards (32 bit)
CPU_TYPE:=cortex-a7
CPU_SUBTYPE:=neon-vfpv4

define Target/Description
	Build firmware image for BCM2709/BCM2710/BCM2711 devices.
	This firmware features a 32 bit kernel.
endef
