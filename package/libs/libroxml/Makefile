#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libroxml
PKG_VERSION:=3.0.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://download.libroxml.net/pool/v3.x
PKG_HASH:=ed6d68d1bceabf98e5e76907411e2e4d93b2dbd48479ab41dede851f59dad6a3
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
  ABI_VERSION:=3.0.2
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
