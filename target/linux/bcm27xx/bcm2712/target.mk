# SPDX-License-Identifier: GPL-2.0-only

ARCH:=aarch64
SUBTARGET:=bcm2712
BOARDNAME:=BCM2712 boards (64 bit)
CPU_TYPE:=cortex-a76
FEATURES+=pci pcie

define Target/Description
	Build firmware image for BCM2712 devices.
	This firmware features a 64 bit kernel.
endef
