#
# Copyright (C) 2017 Ian Li <OpenSource@ianli.xyz>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-transparent-proxy
PKG_VERSION:=0.1.0
PKG_RELEASE:=1

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Ian Li <OpenSource@ianli.xyz>

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/luci-app-transparent-proxy
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=LuCI helper for iptables transparent proxy.
	PKGARCH:=all
	DEPENDS:=+iptables +ipset
endef

define Package/luci-app-transparent-proxy/description
	LuCI helper for iptables transparent proxy.
endef

define Build/Prepare
	$(foreach po,$(wildcard ${CURDIR}/files/luci/i18n/*.po), \
		po2lmo $(po) $(PKG_BUILD_DIR)/$(patsubst %.po,%.lmo,$(notdir $(po)));)
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/luci-app-transparent-proxy/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	if [ -f /etc/uci-defaults/luci-transparent-proxy ]; then
		( . /etc/uci-defaults/luci-transparent-proxy ) && \
		rm -f /etc/uci-defaults/luci-transparent-proxy
	fi
	rm -rf /tmp/luci-indexcache /tmp/luci-modulecache
fi
exit 0
endef

define Package/luci-app-transparent-proxy/conffiles
/etc/config/transparent-proxy
endef

define Package/luci-app-transparent-proxy/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/transparent-proxy.*.lmo $(1)/usr/lib/lua/luci/i18n/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DATA) ./files/luci/controller/*.lua $(1)/usr/lib/lua/luci/controller/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi/transparent-proxy
	$(INSTALL_DATA) ./files/luci/model/cbi/transparent-proxy/*.lua $(1)/usr/lib/lua/luci/model/cbi/transparent-proxy/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/view/transparent-proxy
	$(INSTALL_DATA) ./files/luci/view/transparent-proxy/*.htm $(1)/usr/lib/lua/luci/view/transparent-proxy/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/root/etc/config/transparent-proxy $(1)/etc/config/transparent-proxy
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/root/etc/init.d/transparent-proxy $(1)/etc/init.d/transparent-proxy
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/root/etc/uci-defaults/luci-transparent-proxy $(1)/etc/uci-defaults/luci-transparent-proxy
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./files/root/usr/bin/transparent-proxy-rules$(2) $(1)/usr/bin/transparent-proxy-rules
endef

$(eval $(call BuildPackage,luci-app-transparent-proxy))
