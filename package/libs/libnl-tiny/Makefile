#
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libnl-tiny
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/libnl-tiny.git
PKG_SOURCE_DATE:=2020-08-05
PKG_SOURCE_VERSION:=c291088f631d1694f7ba0444b59677b194348da8
PKG_MIRROR_HASH:=99bcce12701bb34dadb39689d95c2c5cf1e27719d0ecfd645d3957a8947025ac
CMAKE_INSTALL:=1

PKG_LICENSE:=LGPL-2.1
PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libnl-tiny
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=netlink socket library
  ABI_VERSION:=1
endef

define Package/libnl-tiny/description
 This package contains a stripped down version of libnl
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig $(1)/usr/include/libnl-tiny
	$(CP) $(PKG_INSTALL_DIR)/usr/include/libnl-tiny/* $(1)/usr/include/libnl-tiny
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libnl-tiny.so $(1)/usr/lib/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/libnl-tiny.pc $(1)/usr/lib/pkgconfig
endef

define Package/libnl-tiny/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libnl-tiny.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libnl-tiny))
