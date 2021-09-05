#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lz4
PKG_VERSION:=1.9.3
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/lz4/lz4/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=030644df4611007ff7dc962d981f390361e6c97a34e5cbc393ddfbe019ffe2c1

PKG_MAINTAINER:=Darik Horn <dajhorn@vanadac.com>

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
PKG_BUILD_DEPENDS:=meson/host
PKG_CONFIG_DEPENDS:=CONFIG_PACKAGE_lz4

include $(INCLUDE_DIR)/package.mk
include ../../devel/meson/meson.mk

MESON_BUILD_DIR:=$(PKG_BUILD_DIR)/contrib/meson/openwrt-build

define Package/lz4/Default
  SUBMENU:=Compression
  URL:=https://www.lz4.org/
endef

define Package/liblz4
$(call Package/lz4/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Extremely fast compression
  LICENSE:=BSD-2-Clause
  LICENSE_FILES:=LICENSE lib/LICENSE
  ABI_VERSION:=$(firstword $(subst .,$(space),$(PKG_VERSION)))
  MENU:=1
endef

define Package/liblz4/description
  LZ4 is a compression codec that features a very fast encoder and an
  even faster decoder. This package provides the liblz4 shared library.
endef

define Package/liblz4/config
	source "$(SOURCE)/Config.in"
endef

define Package/lz4
$(call Package/lz4/Default)
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Extremely fast compression
  LICENSE:=GPL-2.0-or-later
  LICENSE_FILES:=LICENSE programs/COPYING
  DEPENDS:=+liblz4
endef

define Package/lz4/description
  LZ4 - Fast real-time compression algorithm.
  This package provides the lz4 binaries.
endef

MESON_ARGS += \
	-Ddebug_level=0 \
	-Dbacktrace=false \
	-Dbin_programs=$(if $(CONFIG_PACKAGE_lz4),true,false) \
	-Dbin_tests=false \
	-Dbin_contrib=false \
	-Dbin_examples=false \
	-Db_lto=false

ifeq ($(CONFIG_LZ4_OPTIMIZE_SPEED),y)
	TARGET_CFLAGS := $(filter-out -O%,$(TARGET_CFLAGS)) -O3
endif

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/liblz4.so* $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/liblz4.pc $(1)/usr/lib/pkgconfig
endef

define Package/liblz4/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/liblz4.so.$(ABI_VERSION)* $(1)/usr/lib/
endef

define Package/lz4/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/{lz4,lz4c,lz4cat,unlz4} $(1)/usr/bin/
endef

$(eval $(call BuildPackage,liblz4))
$(eval $(call BuildPackage,lz4))
