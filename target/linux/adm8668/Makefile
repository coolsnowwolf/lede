#
# Copyright (C) 2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=mipsel
BOARD:=adm8668
BOARDNAME:=Infineon WildPass ADM8668
FEATURES:=squashfs usb pci
MAINTAINER:=Florian Fainelli <florian@openwrt.org>

KERNEL_PATCHVER:=3.18

define Target/Description
	Build firmware images for Infineon WildPass (ADM8668) based routers
	(e.g. T-Mobile branded Linksys WRTU54G-TM)
endef

include $(INCLUDE_DIR)/target.mk

$(eval $(call BuildTarget))
