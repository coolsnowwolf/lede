#
# Copyright (C) 2009 OpenWrt.org
#

ARCH:=arm
# BOARD:=meson
# BOARDNAME:=Meson
FEATURES+=fpu usb ext4 display rtc
SUBTARGET:=m8b
BOARDNAME:=Meson8b based boards
# ARCH_PACKAGES:=
CPU_TYPE:=cortex-a5
CPU_SUBTYPE:=neon-vfpv4

KERNELNAME:=Image dtbs

define Target/Description
	Build firmware images for Meson8b based boards.
endef
