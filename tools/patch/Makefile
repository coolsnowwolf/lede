# 
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=patch
PKG_VERSION:=2.7.6
PKG_RELEASE:=6
PKG_CPE_ID:=cpe:/a:gnu:patch

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/patch
PKG_HASH:=ac610bda97abe0d9f6b7c963255a11dcb196c25e337c61f94e4778d632f1d8fd

HOST_BUILD_PARALLEL := 1

include $(INCLUDE_DIR)/host-build.mk

HOSTCC := $(HOSTCC_NOCACHE)
HOSTCXX := $(HOSTCXX_NOCACHE)

define Host/Uninstall
	-$(call Host/Compile/Default,uninstall)
endef

$(eval $(call HostBuild))
