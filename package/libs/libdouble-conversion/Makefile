#
# Copyright (C) 2008-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libdouble-conversion
PKG_VERSION:=2.0
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=0d25506a2e7fb52928963313343e0237e890059e
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_MIRROR_HASH:=7466f634421f3136174cbc2b00635788509a3312894de28367f31d891fd401ad
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=https://github.com/google/double-conversion.git
PKG_LICENSE_FILES:=COPYING LICENSE

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libdouble-conversion
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Efficient binary-decimal and decimal-binary conversion routines for IEEE doubles
  URL:=https://github.com/google/double-conversion
  DEPENDS:=+libstdcpp
endef

define Package/libdouble-conversion/description
  double-conversion provides binary-decimal and decimal-binary routines for IEEE doubles.
  
  The library consists of efficient conversion routines that have been extracted
  from the V8 JavaScript engine. The code has been refactored and improved so that
  it can be used more easily in other projects.
endef

CMAKE_OPTIONS += \
  -DBUILD_SHARED_LIBS=ON \
  -DBUILD_TESTING=OFF

define Build/InstallDev
	$(INSTALL_DIR) \
	  $(1)/usr/lib \
	  $(1)/usr/include/double-conversion

	$(CP) \
	  $(PKG_INSTALL_DIR)/usr/include/* \
	  $(1)/usr/include/

	$(INSTALL_DATA) \
	  $(PKG_INSTALL_DIR)/usr/lib/lib*.so* \
	  $(1)/usr/lib/
endef

define Package/libdouble-conversion/install
	$(INSTALL_DIR) \
	  $(1)/usr/lib

	$(INSTALL_DATA) \
	  $(PKG_INSTALL_DIR)/usr/lib/lib*.so* \
	  $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libdouble-conversion))
