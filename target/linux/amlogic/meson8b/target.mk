# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2006-2013 OpenWrt.org

ARCH:=arm
SUBTARGET:=meson8b
BOARDNAME:=Amlogic S805 boards (32 bit)
CPU_TYPE:=cortex-a5
CPU_SUBTYPE:=vfpv4

define Target/Description
	Build firmware image for Amlogic S805 SoC devices.
	This firmware features a 32 bit kernel.
endef
