#
# Copyright (C) 2008-2014 The LuCI Team <luci@lists.subsignal.org>
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk
PKG_NAME:=luci-app-jd-dailybonus
LUCI_PKGARCH:=all
PKG_VERSION:=0.8.3
PKG_RELEASE:=20200804

include $(INCLUDE_DIR)/package.mk

define Package/luci-app-jd-dailybonus
 	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=Luci for JD dailybonus Script 
	PKGARCH:=all
	DEPENDS:=+node +node-request +coreutils +coreutils-nohup +wget
endef

define Build/Prepare
endef

define Build/Compile
endef

define Package/luci-app-define Package/jd-dailybonus/conffiles

endef

define Package/luci-app-jd-dailybonus/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci
	cp -pR ./luasrc/* $(1)/usr/lib/lua/luci
	$(INSTALL_DIR) $(1)/
	cp -pR ./root/* $(1)/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	po2lmo ./po/zh-cn/jd-dailybonus.po $(1)/usr/lib/lua/luci/i18n/jd-dailybonus.zh-cn.lmo
endef

define Package/luci-app-jd-dailybonus/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	( . /etc/uci-defaults/luci-jd-dailybonus ) && rm -f /etc/uci-defaults/luci-jd-dailybonus
	rm -rf /tmp/luci-indexcache
	rm -rf /tmp/luci-modulecache/*
	chmod +x /etc/init.d/jd-dailybonus >/dev/null 2>&1
	chmod +x /usr/share/jd-dailybonus/newapp.sh >/dev/null 2>&1
	/etc/init.d/jd-dailybonus enable >/dev/null 2>&1
fi
exit 0
endef

define Package/luci-app-jd-dailybonus/prerm
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
     /etc/init.d/jd-dailybonus disable
     /etc/init.d/jd-dailybonus stop
fi
exit 0
endef

$(eval $(call BuildPackage,luci-app-jd-dailybonus))

# call BuildPackage - OpenWrt buildroot signature
