#
# Copyright (C) 2011-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libp11
PKG_VERSION:=0.4.11
PKG_RELEASE:=1

PKG_SOURCE_URL:=https://github.com/OpenSC/libp11/releases/download/$(PKG_NAME)-$(PKG_VERSION)/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=57d47a12a76fd92664ae30032cf969284ebac1dfc25bf824999d74b016d51366

PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING

PKG_BUILD_PARALLEL:=1
PKG_FIXUP:=libtool
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libp11
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=PKCS#11 wrapper library
  URL:=https://www.opensc-project.org/opensc/wiki/libp11
  DEPENDS:=+libopenssl +@OPENSSL_ENGINE
  CONFLICTS:=engine_pkcs11
endef

define Package/libp11/description
  Libp11 is a library implementing a small layer on top of PKCS#11 API
  to make using PKCS#11 implementations easier.
endef

CONFIGURE_ARGS += --with-enginesdir=/usr/lib/engines

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libp11.{a,so} $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libp11.so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libp11.pc $(1)/usr/lib/pkgconfig/libp11.pc
endef

define Package/libp11/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libp11.so.* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/engines
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/engines/*.so* $(1)/usr/lib/engines
	$(LN) pkcs11.so $(1)/usr/lib/engines/libpkcs11.so
endef

$(eval $(call BuildPackage,libp11))
