#
# Copyright 2018 NXP
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

ARCH:=arm
BOARDNAME:=ARMv7 based boards
CPU_TYPE:=cortex-a7
CPU_SUBTYPE:=neon-vfpv4
KERNELNAME:=zImage dtbs

define Target/Description
	Build firmware images for NXP Layerscape ARMv7 based boards.
endef
