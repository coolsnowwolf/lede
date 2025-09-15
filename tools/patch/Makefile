# 
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=patch
PKG_VERSION:=2.7.6
PKG_RELEASE:=7
PKG_CPE_ID:=cpe:/a:gnu:patch

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@GNU/patch
PKG_HASH:=8cf86e00ad3aaa6d26aca30640e86b0e3e1f395ed99f189b06d4c9f74bc58a4e

HOST_BUILD_PARALLEL := 1

include $(INCLUDE_DIR)/host-build.mk

HOSTCC := $(HOSTCC_NOCACHE)
HOSTCXX := $(HOSTCXX_NOCACHE)

define Host/Uninstall
	-$(call Host/Compile/Default,uninstall)
endef

$(eval $(call HostBuild))
