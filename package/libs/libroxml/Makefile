#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libroxml
PKG_VERSION:=2.3.0
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://download.libroxml.net/pool/v2.x
PKG_HASH:=1da8f20b530eba4409f2b217587d2f1281ff5d9ba45b24aeac71b94c6c621b78
PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
PKG_LICENSE:=LGPL-2.1+

include $(INCLUDE_DIR)/package.mk

define Package/libroxml
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Minimum, easy-to-use, C implementation for xml file parsing
  URL:=http://www.libroxml.net/
endef

CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static \
	--disable-roxml

define Build/InstallDev
	$(INSTALL_DIR) $(1)
	$(CP) $(PKG_INSTALL_DIR)/* $(1)/
endef

define Package/libroxml/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libroxml.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libroxml))
