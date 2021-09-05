#
# Copyright (C) 2019-2021 CZ.NIC z.s.p.o. (http://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libmaxminddb
PKG_VERSION:=1.6.0
PKG_RELEASE=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/maxmind/libmaxminddb/releases/download/$(PKG_VERSION)/
PKG_HASH:=7620ac187c591ce21bcd7bf352376a3c56a933e684558a1f6bef4bd4f3f98267

PKG_MAINTAINER:=Jan Pavlinec <jan.pavlinec@nic.cz>
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libmaxminddb
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=A library for working with MaxMind DB files
  URL:=https://maxmind.github.io/libmaxminddb/
endef

define Package/libmaxminddb/description
 The libmaxminddb library provides functions for working MaxMind DB files.
endef

CMAKE_OPTIONS += \
	-DBUILD_SHARED_LIBS=ON \
	-DBUILD_TESTING=OFF

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmaxminddb.so* $(1)/usr/lib/
endef

define Package/libmaxminddb/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/mmdblookup $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmaxminddb.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libmaxminddb))
