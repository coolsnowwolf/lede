#
# Copyright (C) 2016 OpenWrt.org
# Copyright (C) 2017 LEDE project
#

ARCH:=aarch64
SUBTARGET:=bcm2710
BOARDNAME:=BCM2710 64 bit based boards
CPU_TYPE:=cortex-a53

define Target/Description
	Build firmware image for Broadcom BCM2710 64 bit SoC devices.
endef
