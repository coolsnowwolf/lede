#
# Copyright (C) 2019 OpenWrt.org
#

ARCH:=aarch64
SUBTARGET:=bcm2711
BOARDNAME:=BCM2711 boards (64 bit)
CPU_TYPE:=cortex-a72

define Target/Description
	Build firmware image for BCM2711 devices.
	This firmware features a 64 bit kernel.
endef
