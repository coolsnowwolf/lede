#
# Copyright (C) 2015 OpenWrt.org
# Copyright (C) 2017 LEDE project
#

SUBTARGET:=bcm2709
BOARDNAME:=BCM2709/BCM2710 32 bit based boards
CPU_TYPE:=cortex-a7
CPU_SUBTYPE:=neon-vfpv4

define Target/Description
	Build firmware image for Broadcom BCM2709/BCM2710 32 bit SoC devices.
endef
