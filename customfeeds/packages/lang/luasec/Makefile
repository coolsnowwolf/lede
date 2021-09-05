#
# Copyright (C) 2009-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luasec
PKG_VERSION:=0.9
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/brunoos/luasec/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=6b6b94e8517bf6baf545fad29a2112f9ac7957ad85b4aae8e0727bec77d7a325

MAINTAINER:=W. Michael Petullo <mike@flyn.org>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/luasec
  SUBMENU:=Lua
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=LuaSec
  URL:=https://github.com/brunoos/luasec
  DEPENDS:=+lua +libopenssl +luasocket
endef

define Package/luasec/description
  LuaSec is a binding for OpenSSL library to provide TLS/SSL communication.
endef

define Build/Configure
endef

TARGET_CFLAGS += $(FPIC)
TARGET_LDFLAGS += $(FPIC)

MAKE_FLAGS += \
	LD="$(TARGET_CC)" \
	INC_PATH="" \
	LIB_PATH="" \
	LUACPATH="$(PKG_INSTALL_DIR)/usr/lib/lua" \
	LUAPATH="$(PKG_INSTALL_DIR)/usr/lib/lua"

define Build/Compile
$(call Build/Compile/Default,linux)
endef

define Package/luasec/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/lua/ssl.so $(1)/usr/lib/lua/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/lua/ssl.lua $(1)/usr/lib/lua/
	$(INSTALL_DIR) $(1)/usr/lib/lua/ssl
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/lua/ssl/https.lua $(1)/usr/lib/lua/ssl/
endef

$(eval $(call BuildPackage,luasec))
