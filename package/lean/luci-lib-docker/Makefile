#
# Copyright (C) 2019 lisaac <lisaac.cn@gmail.com>
#
# This is free software, licensed under the Apache License, Version 2.0 .
#
include $(TOPDIR)/rules.mk

PKG_NAME:=luci-lib-docker
PKG_VERSION:=v0.1.1
PKG_RELEASE:=beta
PKG_MAINTAINER:=lisaac <https://github.com/lisaac/luci-lib-docker>
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_LICENSE:=Apache-2.0

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=6. Libraries
	TITLE:=Docker Engine API for LuCI
	PKGARCH:=all
	DEPENDS:=+luci-lib-json
endef

define Package/$(PKG_NAME)/description
	Docker Engine API for LuCI
endef

define Build/Prepare
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/postinst
#!/bin/sh
rm -fr /tmp/luci-indexcache /tmp/luci-modulecache
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci
	cp -pR ./luasrc/* $(1)/usr/lib/lua/luci
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
