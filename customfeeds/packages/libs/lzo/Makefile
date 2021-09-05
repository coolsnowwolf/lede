#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lzo
PKG_VERSION:=2.10
PKG_RELEASE:=4

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.oberhumer.com/opensource/lzo/download/
PKG_HASH:=c0f892943208266f9b6543b3ae308fab6284c5c90e627931446fb49b4221a072

PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

CMAKE_BINARY_SUBDIR:=openwrt-build

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/liblzo
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=A real-time data compression library
  URL:=http://www.oberhumer.com/opensource/lzo/
  ABI_VERSION:=2
endef

define Package/liblzo/description
 LZO is a data compression library which is suitable for data de-/compression
 in real-time. This means it favours speed over compression ratio.
endef

CMAKE_OPTIONS += \
	-DENABLE_SHARED=ON \
	-DENABLE_STATIC=ON \
	\
	-DBUILD_TESTING=OFF \
	-DCMAKE_SKIP_INSTALL_RPATH=ON

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include $(1)/usr/lib $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/include/lzo $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/liblzo2.{a,so*} $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/lzo2.pc $(1)/usr/lib/pkgconfig
	$(SED) 's,/usr/include,$$$${prefix}/include,g' $(1)/usr/lib/pkgconfig/lzo2.pc
	$(SED) 's,/usr/lib,$$$${exec_prefix}/lib,g' $(1)/usr/lib/pkgconfig/lzo2.pc
endef

define Package/liblzo/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/liblzo2.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,liblzo))
