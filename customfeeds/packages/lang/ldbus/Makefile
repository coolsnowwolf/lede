#
# Copyright (C) 2006-2017 OpenWrt.org
#
# This is free software, licensed under the GPL 2 license.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ldbus
PKG_SOURCE_DATE:=2019-08-15
PKG_SOURCE_VERSION:=9e176fe851006037a643610e6d8f3a8e597d4073
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/daurnimator/ldbus
PKG_MIRROR_HASH:=65fc7c975a17cfd3929b54973558205354fc285b6bd88a732c045e8444000d44

PKG_MAINTAINER:=Enrico Mioso <mrkiko.rs@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DEPENDS:=luarocks/host
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/ldbus
  SUBMENU:=Lua
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=ldbus Lua DBus bindings
  URL:=https://github.com/daurnimator/ldbus
  DEPENDS:=+dbus +lua
endef

define Package/ldbus/description
  This package contains "ldbus": LUA bindings to interact with the DBUS
  message bus system, and services connected to it.
  See https://github.com/daurnimator/ldbus
  for details.
endef

define Build/Compile
  cd $(PKG_BUILD_DIR) && \
  luarocks make --pack-binary-rock ldbus-scm-0.rockspec \
    LUA_LIBDIR=$(STAGING_DIR)/usr/lib/lua \
    LUA_PKGNAME=lua5.1 \
    DBUS_INCDIR=$(STAGING_DIR)/usr/include/dbus-1.0/ \
    DBUS_ARCH_INCDIR=$(STAGING_DIR)/usr/lib/dbus-1.0/include \
    DBUS_LIBDIR=$(STAGING_DIR)/usr/lib \
    CC="$(TARGET_CC)" LD="$(TARGET_CC)" \
    CFLAGS="$(TARGET_CFLAGS) $(FPIC)" \
    LDFLAGS="$(TARGET_LDFLAGS)"
endef

define Package/ldbus/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ldbus.so $(1)/usr/lib/lua
endef

$(eval $(call BuildPackage,ldbus))
