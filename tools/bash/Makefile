#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=bash
PKG_CPE_ID:=cpe:/a:gnu:bash
PKG_VERSION:=5.2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@GNU/bash
PKG_HASH:=a139c166df7ff4471c5e0733051642ee5556c1cc8a4a78f145583c5c81ab32fb

HOST_BUILD_PARALLEL := 1

include $(INCLUDE_DIR)/host-build.mk

define Host/Uninstall
	-$(call Host/Compile/Default,uninstall)
endef

$(eval $(call HostBuild))
