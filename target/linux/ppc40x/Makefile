#
# Copyright (C) 2008-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=powerpc
BOARD:=ppc40x
BOARDNAME:=AMCC/IBM PPC40x
FEATURES:=squashfs small_flash broken
CPU_TYPE:=405
MAINTAINER:=Imre Kaloz <kaloz@openwrt.org>

KERNEL_PATCHVER:=3.18

define Target/Description
	Build firmware images for AMCC/IBM PPC40x based boards.
endef

include $(INCLUDE_DIR)/target.mk

KERNELNAME:=uImage cuImage.magicbox cuImage.openrb

$(eval $(call BuildTarget))
