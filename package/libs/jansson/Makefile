#
# Copyright (C) 2011-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=jansson
PKG_VERSION:=2.12
PKG_RELEASE:=1
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://www.digip.org/jansson/releases/
PKG_HASH:=645d72cc5dbebd4df608d33988e55aa42a7661039e19a379fcbe5c79d1aee1d2

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

CMAKE_INSTALL:=1
CMAKE_OPTIONS += \
	-DJANSSON_BUILD_DOCS:BOOL=OFF \
	-DJANSSON_BUILD_SHARED_LIBS:BOOL=ON \
	-DJANSSON_EXAMPLES:BOOL=OFF \
	-DJANSSON_WITHOUT_TESTS:BOOL=ON

define Package/jansson
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Jansson library
  URL:=http://www.digip.org/jansson/
endef

define Package/jansson/description
  Jansson is a C library for encoding, decoding and manipulating JSON data
endef

define Package/jansson/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libjansson*so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,jansson))
