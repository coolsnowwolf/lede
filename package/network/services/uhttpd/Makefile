#
# Copyright (C) 2010-2015 Jo-Philipp Wich <jo@mein.io>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=uhttpd
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(LEDE_GIT)/project/uhttpd.git
PKG_SOURCE_DATE:=2016-10-25
PKG_SOURCE_VERSION:=1628fa4b34aa143187353f81e8001b9a15286bda
PKG_MIRROR_HASH:=2ac4ba8dc0b349d72174aac9ff693a73a214295a9890fe3d2a8eedcad54d06e3
PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>
PKG_LICENSE:=ISC

PKG_BUILD_DEPENDS = ustream-ssl

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk
include $(INCLUDE_DIR)/version.mk

define Package/uhttpd/default
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Web Servers/Proxies
  TITLE:=uHTTPd - tiny, single threaded HTTP server
endef

define Package/uhttpd
  $(Package/uhttpd/default)
  DEPENDS:=+libubox +libblobmsg-json +libjson-script
endef

define Package/uhttpd/description
 uHTTPd is a tiny single threaded HTTP server with TLS, CGI and Lua
 support. It is intended as a drop-in replacement for the Busybox
 HTTP daemon.
endef

define Package/uhttpd/config
  config PACKAGE_uhttpd_debug
    bool "Build with debug messages"
    default n
  config uhttpd_lua
    depends on PACKAGE_uhttpd-mod-lua
    bool "Enable Integrated Lua interpreter"
	default y
endef

define Package/uhttpd-mod-lua
  $(Package/uhttpd/default)
  TITLE+= (Lua plugin)
  DEPENDS:=uhttpd +liblua
endef

define Package/uhttpd-mod-lua/description
 The Lua plugin adds a CGI-like Lua runtime interface to uHTTPd.
endef


define Package/uhttpd-mod-ubus
  $(Package/uhttpd/default)
  TITLE+= (ubus plugin)
  DEPENDS:=uhttpd +libubus +libblobmsg-json
endef

define Package/uhttpd-mod-ubus/description
 The ubus plugin adds a HTTP/JSON RPC proxy for ubus and publishes the
 session.* namespace and procedures.
endef

define Package/uhttpd/conffiles
/etc/config/uhttpd
/etc/uhttpd.crt
/etc/uhttpd.key
endef

ifneq ($(CONFIG_USE_GLIBC),)
  TARGET_CFLAGS += -D_DEFAULT_SOURCE
endif

TARGET_LDFLAGS += -lcrypt

CMAKE_OPTIONS = -DTLS_SUPPORT=on

define Package/uhttpd/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/uhttpd.init $(1)/etc/init.d/uhttpd
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/uhttpd.config $(1)/etc/config/uhttpd
	$(VERSION_SED_SCRIPT) $(1)/etc/config/uhttpd
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/uhttpd $(1)/usr/sbin/uhttpd
endef

define Package/uhttpd-mod-lua/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/uhttpd_lua.so $(1)/usr/lib/
endef

define Package/uhttpd-mod-ubus/install
	$(INSTALL_DIR) $(1)/usr/lib $(1)/etc/uci-defaults
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/uhttpd_ubus.so $(1)/usr/lib/
	$(INSTALL_DATA) ./files/ubus.default $(1)/etc/uci-defaults/00_uhttpd_ubus
endef

define Package/uhttpd-mod-lua/postinst
	$(if $(CONFIG_uhttpd_lua),
	#!/bin/sh
	if [ -f $${IPKG_INSTROOT}/www/index.html ]
	then
		sed -e 's:/cgi-bin::g' -i $${IPKG_INSTROOT}/www/index.html
	fi
	,)
endef


$(eval $(call BuildPackage,uhttpd))
$(eval $(call BuildPackage,uhttpd-mod-lua))
$(eval $(call BuildPackage,uhttpd-mod-ubus))
