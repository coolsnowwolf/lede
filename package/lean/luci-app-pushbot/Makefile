# Copyright (C) 2020 Openwrt.org
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-pushbot
PKG_VERSION:=3.12
PKG_RELEASE:=9

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=luci
  CATEGORY:=LuCI
  SUBMENU:=3. Applications
  DEPENDS:=+iputils-arping +curl
  TITLE:=LuCI support for Pushbot
  PKGARCH:=all
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/conffiles
/etc/config/pushbot
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc/init.d $(1)/usr/bin/pushbot $(1)/etc/config $(1)/usr/lib/lua/luci $(1)/etc/uci-defaults $(1)/usr/share/rpcd/acl.d
	$(CP) ./luasrc/* $(1)/usr/lib/lua/luci
	$(INSTALL_CONF) ./root/etc/config/pushbot $(1)/etc/config
	$(INSTALL_BIN) ./root/etc/init.d/pushbot $(1)/etc/init.d
	$(INSTALL_BIN) ./root/etc/uci-defaults/luci-pushbot $(1)/etc/uci-defaults/luci-pushbot
	$(INSTALL_BIN) ./root/usr/bin/pushbot/pushbot $(1)/usr/bin/pushbot
	$(INSTALL_DATA) ./root/usr/share/rpcd/acl.d/luci-app-pushbot.json $(1)/usr/share/rpcd/acl.d/luci-app-pushbot.json
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
