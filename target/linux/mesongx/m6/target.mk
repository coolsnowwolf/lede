#
# Copyright (C) 2009 OpenWrt.org
#

SUBTARGET:=m6
BOARDNAME:=Meson6 based boards
ARCH_PACKAGES:=
FEATURES+=
CPU_TYPE:=cortex-a9
CPU_SUBTYPE:=neon
Kernel/Patch:=$(Kernel/Patch/Amlogic)

define Target/Description
	Build firmware images for Meson6 based boards.
endef

