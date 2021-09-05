#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=dtndht
PKG_VERSION:=0.2.3
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.ibr.cs.tu-bs.de/projects/ibr-dtn/releases
PKG_HASH:=29749f6c4e1b79e6973dc7436bcc6e0aee86ce27e1786f32a60feab652af8898
PKG_MAINTAINER:=Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
PKG_LICENSE:=Apache-2.0

PKG_INSTALL:=1
PKG_FIXUP:=libtool

include $(INCLUDE_DIR)/package.mk

define Package/dtndht
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libopenssl
  TITLE:=Library for DHT lookups for DTNs
endef

define Package/dtndht/description
 A library with DHT functions for DTN name lookups.
endef

TARGET_CFLAGS += -std=gnu89

define Build/InstallDev
	$(INSTALL_DIR) $(1)
	$(CP) $(PKG_INSTALL_DIR)/* $(1)/
endef

define Package/dtndht/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,dtndht))
