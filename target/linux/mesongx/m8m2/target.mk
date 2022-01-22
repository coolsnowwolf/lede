#
# Copyright (C) 2009 OpenWrt.org
#

SUBTARGET:=m8m2
BOARDNAME:=Meson8m2 based boards
ARCH_PACKAGES:=
FEATURES+=
CPU_TYPE:=cortex-a9
CPU_SUBTYPE:=neon
Kernel/Patch:=$(Kernel/Patch/Amlogic)

define Target/Description
	Build firmware images for Meson8m2 based boards.
endef

