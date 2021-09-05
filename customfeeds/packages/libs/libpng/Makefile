#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libpng
PKG_VERSION:=1.6.37
PKG_RELEASE:=11

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@SF/libpng
PKG_HASH:=505e70834d35383537b6491e7ae8641f1a4bed1876dbfe361201fc80868d88ca

PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>
PKG_LICENSE:=Libpng GPL-2.0-or-later BSD-3-Clause
PKC_LICENSE_FILES:=LICENSE contrib/gregbook/COPYING contrib/gregbook/LICENSE
PKG_CPE_ID:=cpe:/a:libpng:libpng

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libpng
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+zlib
  TITLE:=A PNG format files handling library
  URL:=http://www.libpng.org/pub/png/libpng.html
endef

CMAKE_OPTIONS += \
	-DCMAKE_POSITION_INDEPENDENT_CODE=ON \
	-DPNG_BUILD_ZLIB=OFF \
	-DPNG_SHARED=ON \
	-DPNG_STATIC=ON \
	-DPNG_EXECUTABLES=OFF \
	-DPNG_TESTS=OFF \
	-DPNG_FRAMEWORK=OFF \
	-DPNG_DEBUG=OFF \
	-DPNG_HARDWARE_OPTIMIZATIONS=O$(if $(findstring powerpc,$(CONFIG_ARCH))$(findstring mips,$(CONFIG_ARCH)),FF,N) \
	-Dld-version-script=OFF

define Build/InstallDev
	$(call Build/InstallDev/cmake,$(1))
	$(SED) 's,^\(prefix\|exec_prefix\)=.*,\1=$(STAGING_DIR)/usr,g' $(1)/usr/bin/libpng{,16}-config
	$(SED) '/^includedir=/s|/usr|$$$${prefix}|' $(1)/usr/bin/libpng{,16}-config
	$(SED) '/^libdir=/s|/usr|$$$${prefix}|' $(1)/usr/bin/libpng{,16}-config
	$(SED) 's,/usr/include,$$$${prefix}/include,g' $(1)/usr/lib/pkgconfig/libpng16.pc
	$(SED) 's,/usr/lib,$$$${exec_prefix}/lib,g' $(1)/usr/lib/pkgconfig/libpng16.pc
	$(INSTALL_DIR) $(2)/bin
	for f in libpng{,16}-config; do \
		$(LN) ../../usr/bin/$$$$f $(2)/bin/ ; \
	done
endef

define Package/libpng/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libpng16.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libpng.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libpng))
