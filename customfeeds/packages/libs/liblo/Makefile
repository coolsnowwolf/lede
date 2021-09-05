#
# Copyright (C) 2010-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=liblo
PKG_VERSION:=0.31
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/liblo
PKG_HASH:=2b4f446e1220dcd624ecd8405248b08b7601e9a0d87a0b94730c2907dbccc750

PKG_MAINTAINER:=
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING

CMAKE_INSTALL:=1
CMAKE_SOURCE_SUBDIR:=cmake
PKG_CONFIG_DEPENDS:=CONFIG_PACKAGE_liblo-utils

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/liblo/Default
  TITLE:=Lightweight Open Sound Control (OSC)
  URL:=http://liblo.sourceforge.net/
endef

define Package/liblo
$(call Package/liblo/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  SUBMENU:=Sound
  TITLE+= library
  DEPENDS:= +libpthread
endef

define Package/liblo-utils
$(call Package/liblo/Default)
  CATEGORY:=Sound
  TITLE+= utilities
  DEPENDS:= +liblo
endef

CMAKE_OPTIONS += \
	-DWITH_TOOLS=O$(if $(CONFIG_PACKAGE_liblo-utils),N,FF) \
	-DWITH_TESTS=OFF \
	-DWITH_EXAMPLES=OFF \
	-DWITH_CPP_TESTS=OFF \
	-DWITH_STATIC=OFF \
	-DTHREADING=ON

define Package/liblo/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/liblo.so.* $(1)/usr/lib/
endef

define Package/liblo-utils/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/osc{dump,send} $(1)/usr/bin/
endef

$(eval $(call BuildPackage,liblo))
$(eval $(call BuildPackage,liblo-utils))
