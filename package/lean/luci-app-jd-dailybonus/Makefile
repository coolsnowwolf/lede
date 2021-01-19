#
# Copyright (C) 2008-2014 The LuCI Team <luci@lists.subsignal.org>
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk
PKG_NAME:=luci-app-jd-dailybonus
LUCI_PKGARCH:=all
PKG_VERSION:=0.8.9
PKG_RELEASE:=20201230

include $(INCLUDE_DIR)/package.mk

define Package/luci-app-jd-dailybonus
 	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=Luci for JD dailybonus Script 
	PKGARCH:=all
	DEPENDS:=+node +wget
endef

define Build/Prepare
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/conffiles
/etc/config/jd-dailybonus
endef

define Package/luci-app-jd-dailybonus/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./root/etc/config/jd-dailybonus $(1)/etc/config/jd-dailybonus

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./root/etc/init.d/* $(1)/etc/init.d/

	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./root/etc/uci-defaults/* $(1)/etc/uci-defaults/

	$(INSTALL_DIR) $(1)/usr/share/jd-dailybonus
	$(INSTALL_BIN) ./root/usr/share/jd-dailybonus/*.sh $(1)/usr/share/jd-dailybonus/
	$(INSTALL_DATA) ./root/usr/share/jd-dailybonus/*.js $(1)/usr/share/jd-dailybonus/

	$(INSTALL_DIR) $(1)/usr/share/rpcd/acl.d
	$(INSTALL_DATA) ./root/usr/share/rpcd/acl.d/* $(1)/usr/share/rpcd/acl.d

	$(INSTALL_DIR) $(1)/usr/lib/node
	cp -pR ./root/usr/lib/node/* $(1)/usr/lib/node

	$(INSTALL_DIR) $(1)/www/jd-dailybonus
	$(INSTALL_DATA) ./root//www/jd-dailybonus/* $(1)/www/jd-dailybonus

	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DATA) ./luasrc/controller/* $(1)/usr/lib/lua/luci/controller/

	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi/jd-dailybonus
	$(INSTALL_DATA) ./luasrc/model/cbi/jd-dailybonus/* $(1)/usr/lib/lua/luci/model/cbi/jd-dailybonus/

	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/view/jd-dailybonus
	$(INSTALL_DATA) ./luasrc/view/jd-dailybonus/* $(1)/usr/lib/lua/luci/view/jd-dailybonus/

	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	po2lmo ./po/zh-cn/jd-dailybonus.po $(1)/usr/lib/lua/luci/i18n/jd-dailybonus.zh-cn.lmo
endef

$(eval $(call BuildPackage,luci-app-jd-dailybonus))

# call BuildPackage - OpenWrt buildroot signature
