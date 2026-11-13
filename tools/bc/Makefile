# 
# Copyright (C) 2013-2022 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=bc
PKG_VERSION:=1.08.2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@GNU/bc
PKG_HASH:=ae470fec429775653e042015edc928d07c8c3b2fc59765172a330d3d87785f86

PKG_CPE_ID:=cpe:/a:gnu:bc

PKG_SUBDIRS:=lib bc dc

HOST_CONFIGURE_CACHE:=1

include $(INCLUDE_DIR)/host-build.mk

HOST_MAKE_FLAGS += SUBDIRS="$(PKG_SUBDIRS)"

define Host/Uninstall
	-$(call Host/Compile/Default,uninstall SUBDIRS='lib bc dc')
endef

$(eval $(call HostBuild))
