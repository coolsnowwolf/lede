#
# Copyright (C) 2018 Jianhui Zhao
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libuhttpd
PKG_VERSION:=3.12.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL=https://github.com/zhaojh329/libuhttpd/releases/download/v$(PKG_VERSION)
PKG_HASH:=c234dd3d491c4daa047e28870c6e740529b2eff2dd027dafb6bf02d8dba286b0

PKG_MAINTAINER:=Jianhui Zhao <zhaojh329@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libuhttpd/Default
  SECTION:=libs
  CATEGORY:=Libraries
  SUBMENU:=Networking
  TITLE:=A lightweight HTTP server library based on libev
  URL:=https://github.com/zhaojh329/libuhttpd
  DEPENDS:=+libev $(2)
  VARIANT:=$(1)
  PROVIDES:=libuhttpd
endef

Package/libuhttpd-openssl=$(call Package/libuhttpd/Default,openssl,+PACKAGE_libuhttpd-openssl:libopenssl)
Package/libuhttpd-wolfssl=$(call Package/libuhttpd/Default,wolfssl,+PACKAGE_libuhttpd-wolfssl:libwolfssl)
Package/libuhttpd-mbedtls=$(call Package/libuhttpd/Default,mbedtls,+PACKAGE_libuhttpd-mbedtls:libmbedtls +PACKAGE_libuhttpd-mbedtls:zlib)
Package/libuhttpd-nossl=$(call Package/libuhttpd/Default,nossl)

ifeq ($(BUILD_VARIANT),openssl)
  CMAKE_OPTIONS += -DUSE_OPENSSL=ON
else ifeq ($(BUILD_VARIANT),wolfssl)
  CMAKE_OPTIONS += -DUSE_WOLFSSL=ON
else ifeq ($(BUILD_VARIANT),mbedtls)
  CMAKE_OPTIONS += -DUSE_MBEDTLS=ON
else
  CMAKE_OPTIONS += -DSSL_SUPPORT=OFF
endif

define Package/libuhttpd-$(BUILD_VARIANT)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libuhttpd.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libuhttpd-openssl))
$(eval $(call BuildPackage,libuhttpd-mbedtls))
$(eval $(call BuildPackage,libuhttpd-wolfssl))
$(eval $(call BuildPackage,libuhttpd-nossl))
