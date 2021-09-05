#
# Copyright (C) 2020 Rafał Dzięgiel <rafostar.github@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libdvbcsa
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_VERSION:=bc6c0b164a87ce05e9925785cc6fb3f54c02b026
PKG_HASH:=2d761c9e094642f2c9aa7e66534c6147a59d0d0bc709ec0f2fdbb34bf020d8ec
PKG_SOURCE:=$(PKG_NAME)-$(PKG_SOURCE_VERSION).tar.gz
PKG_SOURCE_URL:=https://code.videolan.org/videolan/libdvbcsa/-/archive/$(PKG_SOURCE_VERSION)
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_SOURCE_VERSION)

PKG_MAINTAINER:=Rafał Dzięgiel <rafostar.github@gmail.com>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libdvbcsa
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=DVB Common Scrambling Algorithm Library
  URL:=https://www.videolan.org/developers/libdvbcsa.html
  DEPENDS:=@BUILD_PATENTED
endef

define Package/libdvbcsa/description
 Libdvbcsa is a free implementation of the DVB Common Scrambling
 Algorithm DVB/CSA - with encryption and decryption capabilities.
endef

define Package/libdvbcsa/config
  menu "Configuration"
  depends on PACKAGE_libdvbcsa
  source "$(SOURCE)/Config.in"
  endmenu
endef

CONFIGURE_ARGS += \
	--$(if $(CONFIG_LIBDVBCSA_DEBUG),en,dis)able-debug \
	--$(if $(CONFIG_LIBDVBCSA_MMX),en,dis)able-mmx \
	--$(if $(CONFIG_LIBDVBCSA_SSE2),en,dis)able-sse2 \
	--$(if $(CONFIG_LIBDVBCSA_ALTIVEC),en,dis)able-altivec \
	--$(if $(CONFIG_LIBDVBCSA_NEON),en,dis)able-neon

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/dvbcsa
	$(CP) $(PKG_INSTALL_DIR)/usr/include/dvbcsa/*.h $(1)/usr/include/dvbcsa/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
endef

define Package/libdvbcsa/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libdvbcsa.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libdvbcsa))
