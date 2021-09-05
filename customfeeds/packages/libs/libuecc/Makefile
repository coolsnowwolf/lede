#
# Copyright (C) 2012-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libuecc
PKG_VERSION:=7
PKG_RELEASE:=2

PKG_MAINTAINER:=Matthias Schiffer <mschiffer@universe-factory.net>
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://github.com/NeoRaider/libuecc/releases/download/v$(PKG_VERSION)
PKG_HASH:=b94aef08eab5359d0facaa7ead2ce81b193eef0c61379d9835213ebc0a46257a

PKG_LICENSE:=BSD-2-Clause
PKG_LICENSE_FILES:=COPYRIGHT

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libuecc
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Very small Elliptic Curve Cryptography library
  URL:=https://github.com/NeoRaider/libuecc/
endef

TARGET_CFLAGS += -ffunction-sections -fdata-sections

CMAKE_OPTIONS += \
	-DCMAKE_BUILD_TYPE:String="MINSIZEREL"


define Package/libuecc/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libuecc.so* $(1)/usr/lib/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/libuecc-$(PKG_VERSION) $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libuecc.so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libuecc.pc $(1)/usr/lib/pkgconfig/
endef

$(eval $(call BuildPackage,libuecc))
