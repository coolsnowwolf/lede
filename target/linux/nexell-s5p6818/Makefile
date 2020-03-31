#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=aarch64
BOARD:=nexell-s5p6818
BOARDNAME:=NEXELL S5P6818 (aarch64)
FEATURES:=targz gpio rtc usb fpu
MAINTAINER:=FriendlyARM <support@friendlyarm.com>
CPU_TYPE:=cortex-a53

KERNEL_PATCHVER:=4.14

define Target/Description
	Build firmware images for NEXELL S5P6818 based FriendlyARM S5P6818 boards routers with ARM CPU, *not* MIPS.
endef

include $(INCLUDE_DIR)/target.mk

KERNELNAME:=Image

$(eval $(call BuildTarget))
