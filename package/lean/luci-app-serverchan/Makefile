# Copyright (C) 2016 Openwrt.org
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-serverchan
PKG_VERSION:=1.80
PKG_RELEASE:=8

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=luci
  CATEGORY:=LuCI
  SUBMENU:=3. Applications
  DEPENDS:=+iputils-arping +curl
  TITLE:=LuCI support for serverchan
  PKGARCH:=all
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/conffiles
/etc/config/serverchan
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc/init.d $(1)/usr/bin/serverchan $(1)/etc/config $(1)/usr/lib/lua/luci $(1)/etc/uci-defaults $(1)/usr/share/rpcd/acl.d
	$(CP) ./luasrc/* $(1)/usr/lib/lua/luci
	$(INSTALL_CONF) ./root/etc/config/serverchan $(1)/etc/config
	$(INSTALL_BIN) ./root/etc/init.d/serverchan $(1)/etc/init.d
	$(INSTALL_BIN) ./root/etc/uci-defaults/luci-serverchan $(1)/etc/uci-defaults/luci-serverchan
	$(INSTALL_BIN) ./root/usr/bin/serverchan/serverchan $(1)/usr/bin/serverchan
	$(INSTALL_DATA) ./root/usr/share/rpcd/acl.d/luci-app-serverchan.json $(1)/usr/share/rpcd/acl.d/luci-app-serverchan.json
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
