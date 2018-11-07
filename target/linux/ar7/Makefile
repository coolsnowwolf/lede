#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=mipsel
BOARD:=ar7
BOARDNAME:=TI AR7
FEATURES:=squashfs atm low_mem
MAINTAINER:=Florian Fainelli <florian@openwrt.org>
SUBTARGETS:=generic ac49x

KERNEL_PATCHVER:=4.9

define Target/Description
	Build firmware images for TI AR7 based routers.
endef

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES+= swconfig

$(eval $(call BuildTarget))
