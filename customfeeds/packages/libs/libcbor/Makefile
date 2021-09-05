#
# Copyright (C) 2020 Linos Giannopoulos
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libcbor
PKG_VERSION:=0.8.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/PJK/libcbor/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=618097166ea4a54499646998ccaa949a5816e6a665cf1d6df383690895217c8b

PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=COPYING
PKG_MAINTAINER:=Linos Giannopoulos <linosgian00+openwrt@gmail.com>

CMAKE_OPTIONS += \
        -DBUILD_SHARED_LIBS=ON
CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libcbor
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=libcbor
  URL:=https://github.com/PJK/libcbor
  ABI_VERSION:=0
endef

define Package/libcbor/description
 libcbor is a C library for parsing and generating CBOR, the general-purpose schema-less binary data format.
endef


define Package/libcbor/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcbor.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libcbor))
