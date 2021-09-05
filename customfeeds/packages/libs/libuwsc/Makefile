#
# Copyright (C) 2018 Jianhui Zhao
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libuwsc
PKG_VERSION:=3.3.5
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL=https://github.com/zhaojh329/libuwsc/releases/download/v$(PKG_VERSION)
PKG_HASH:=a06b7324671e181ffe3165e93e6f94c7ac1380f69e32a52e80c8da7016acd60d

PKG_MAINTAINER:=Jianhui Zhao <jianhuizhao329@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

CMAKE_INSTALL:=1

PKG_CONFIG_DEPENDS:= \
	CONFIG_LIBUWSC_openssl_LUA_BINDING \
	CONFIG_LIBUWSC_wolfssl_LUA_BINDING \
	CONFIG_LIBUWSC_mbedtls_LUA_BINDING \
	CONFIG_LIBUWSC_nossl_LUA_BINDING

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libuwsc/Default
  SECTION:=libs
  CATEGORY:=Libraries
  SUBMENU:=Networking
  TITLE:=A lightweight WebSocket client library based on libev
  URL:=https://github.com/zhaojh329/libuwsc
  DEPENDS:=+libev $(2)
  VARIANT:=$(1)
  PROVIDES:=libuwsc
endef

Package/libuwsc-openssl=$(call Package/libuwsc/Default,openssl,+PACKAGE_libuwsc-openssl:libopenssl)
Package/libuwsc-wolfssl=$(call Package/libuwsc/Default,wolfssl,+PACKAGE_libuwsc-wolfssl:libwolfssl)
Package/libuwsc-mbedtls=$(call Package/libuwsc/Default,mbedtls,+PACKAGE_libuwsc-mbedtls:libmbedtls)
Package/libuwsc-nossl=$(call Package/libuwsc/Default,nossl)

define Package/libuwsc/config
menu "Configuration"
	depends on PACKAGE_libuwsc-$(1)

config LIBUWSC_$(1)_LUA_BINDING
	bool
	default n
	prompt "Lua binding for libuwsc"
endmenu
endef

Package/libuwsc-openssl/config=$(call Package/libuwsc/config,openssl)
Package/libuwsc-wolfssl/config=$(call Package/libuwsc/config,wolfssl)
Package/libuwsc-mbedtls/config=$(call Package/libuwsc/config,mbedtls)
Package/libuwsc-nossl/config=$(call Package/libuwsc/config,nossl)

ifeq ($(BUILD_VARIANT),openssl)
  CMAKE_OPTIONS += -DUWSC_USE_OPENSSL=ON
else ifeq ($(BUILD_VARIANT),wolfssl)
  CMAKE_OPTIONS += -DUWSC_USE_WOLFSSL=ON
else ifeq ($(BUILD_VARIANT),mbedtls)
  CMAKE_OPTIONS += -DUWSC_USE_MBEDTLS=ON
else
  CMAKE_OPTIONS += -DUWSC_SSL_SUPPORT=OFF
endif

ifneq ($(CONFIG_LIBUWSC_$(BUILD_VARIANT)_LUA_BINDING),)
  CMAKE_OPTIONS += -DUWSC_LUA_SUPPORT=ON
endif

define Package/libuwsc-$(BUILD_VARIANT)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libuwsc.so* $(1)/usr/lib/
ifneq ($(CONFIG_LIBUWSC_$(BUILD_VARIANT)_LUA_BINDING),)
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lua/uwsc.so $(1)/usr/lib/lua
endif
endef

$(eval $(call BuildPackage,libuwsc-openssl))
$(eval $(call BuildPackage,libuwsc-wolfssl))
$(eval $(call BuildPackage,libuwsc-mbedtls))
$(eval $(call BuildPackage,libuwsc-nossl))
