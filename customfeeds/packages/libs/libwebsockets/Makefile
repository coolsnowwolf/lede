#
# Copyright (C) 2014-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libwebsockets
PKG_VERSION:=4.1.6
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=https://codeload.github.com/warmcat/libwebsockets/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=402e9a8df553c9cd2aff5d7a9758e9e5285bf3070c82539082864633db3deb83

PKG_SOURCE_VERSION:=v$(PKG_VERSION)

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

CMAKE_OPTIONS += -DLWS_IPV6=$(if $(CONFIG_IPV6),ON,OFF)
CMAKE_OPTIONS += -DISABLE_WERROR=ON
CMAKE_OPTIONS += -DCMAKE_BUILD_TYPE=Release

# turn off all test apps
CMAKE_OPTIONS += -DLWS_WITHOUT_TESTAPPS=ON
CMAKE_OPTIONS += -DLWS_WITHOUT_TEST_SERVER=ON
CMAKE_OPTIONS += -DLWS_WITHOUT_TEST_SERVER_EXTPOLL=ON
CMAKE_OPTIONS += -DLWS_WITHOUT_TEST_PING=ON
CMAKE_OPTIONS += -DLWS_WITHOUT_TEST_CLIENT=ON

define Package/libwebsockets/Default
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=libwebsockets
	DEPENDS:=+zlib +libcap
	URL:=https://libwebsockets.org
	MAINTAINER:=Karl Palsson <karlp@etactica.com>
	PROVIDES:= libwebsockets
endef

define Package/libwebsockets-openssl
	$(call Package/libwebsockets/Default)
	TITLE += (OpenSSL)
	DEPENDS += +libopenssl
	VARIANT:=openssl
endef

define Package/libwebsockets-mbedtls
	$(call Package/$(PKG_NAME)/Default)
	TITLE += (mbedTLS)
	DEPENDS += +libmbedtls
	VARIANT:=mbedtls
endef

define Package/libwebsockets-full
	$(call Package/libwebsockets/Default)
	TITLE += (Full - OpenSSL, libuv, plugins, CGI)
	DEPENDS += +libopenssl +libuv
	VARIANT:=full
endef

ifeq ($(BUILD_VARIANT),openssl)
    CMAKE_OPTIONS += -DLWS_OPENSSL_CLIENT_CERTS=/etc/ssl/certs
    CMAKE_OPTIONS += -DLWS_WITH_SSL=ON
endif

ifeq ($(BUILD_VARIANT),mbedtls)
    CMAKE_OPTIONS += -DLWS_WITH_MBEDTLS=1
endif

ifeq ($(BUILD_VARIANT),full)
    CMAKE_OPTIONS += -DLWS_OPENSSL_CLIENT_CERTS=/etc/ssl/certs
    CMAKE_OPTIONS += -DLWS_WITH_SSL=ON
    CMAKE_OPTIONS += -DLWS_WITH_LIBUV=ON
    CMAKE_OPTIONS += -DLWS_WITH_PLUGINS=ON
    CMAKE_OPTIONS += -DLWS_WITH_SERVER_STATUS=ON
    CMAKE_OPTIONS += -DLWS_WITH_ACCESS_LOG=ON
    CMAKE_OPTIONS += -DLWS_WITH_CGI=ON
    CMAKE_OPTIONS += -DLWS_UNIX_SOCK=ON
endif

define Package/libwebsockets/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libwebsockets.so* $(1)/usr/lib/
endef

Package/libwebsockets-mbedtls/install = $(Package/libwebsockets/install)
Package/libwebsockets-openssl/install = $(Package/libwebsockets/install)

define Package/libwebsockets-full/install
	$(call Package/libwebsockets/install,$(1))
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libwebsockets-evlib_uv.so* $(1)/usr/lib
endef

$(eval $(call BuildPackage,libwebsockets-openssl))
$(eval $(call BuildPackage,libwebsockets-mbedtls))
$(eval $(call BuildPackage,libwebsockets-full))
