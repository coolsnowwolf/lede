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
PKG_SOURCE_DATE:=2021-11-21
PKG_SOURCE_VERSION:=8e0555fb39f51a5d6436b4f1370850caa03611ea
PKG_MIRROR_HASH:=2cfbcc62384535674a2c0157cb24a0736520fcb66ed50be23bf9141c8488885f
CMAKE_INSTALL:=1

PKG_FLAGS := nonshared

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
