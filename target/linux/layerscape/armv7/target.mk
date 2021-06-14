# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright 2018 NXP

ARCH:=arm
BOARDNAME:=ARMv7 based boards
CPU_TYPE:=cortex-a7
CPU_SUBTYPE:=neon-vfpv4
KERNELNAME:=zImage dtbs

define Target/Description
	Build firmware images for NXP Layerscape ARMv7 based boards.
endef
