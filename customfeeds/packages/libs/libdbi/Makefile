#
# Copyright (C) 2009-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libdbi
PKG_VERSION:=0.9.0
PKG_RELEASE:=5

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/libdbi
PKG_HASH:=dafb6cdca524c628df832b6dd0bf8fabceb103248edb21762c02d3068fca4503

PKG_MAINTAINER:=
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libdbi
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Database Independent Abstraction Layer library
  URL:=http://libdbi.sourceforge.net/
endef

define Package/libdbi/description
 This package provides a database-independent abstraction layer library in C.
endef

CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static \
	--disable-docs

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/dbi $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libdbi.{a,so*} $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/dbi.pc $(1)/usr/lib/pkgconfig/
endef

define Package/libdbi/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libdbi.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libdbi))
