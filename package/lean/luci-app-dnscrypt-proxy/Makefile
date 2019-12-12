#
# Copyright (C) 2019 OpenWrt-DNSCrypt-Proxy
# Copyright (C) 2019 peter-tank
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-dnscrypt-proxy
PKG_VERSION:=2.0.1
PKG_RELEASE:=1

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=peter-tank

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)/$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)

PKG_CONFIG_DEPENDS:=CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_minisign

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)/config

config PACKAGE_$(PKG_NAME)_INCLUDE_minisign
	bool "Include minisign for customized offline resolvers list self-sign."
	default n

endef

define Package/luci-app-dnscrypt-proxy
 	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=DNSCrypt Proxy LuCI interface
	URL:=https://github.com/peter-tank/luci-app-dnscrypt-proxy
	PKGARCH:=all
	DEPENDS:=+PACKAGE_$(PKG_NAME)_INCLUDE_minisign:minisign
endef

define Package/luci-app-dnscrypt-proxy/description
	LuCI Support for dnscrypt-proxy2.
endef

define Build/Prepare
	$(foreach po,$(wildcard ${CURDIR}/files/luci/i18n/*.po), \
		po2lmo $(po) $(PKG_BUILD_DIR)/$(patsubst %.po,%.lmo,$(notdir $(po)));)
endef

define Build/Compile
endef

define Package/luci-app-dnscrypt-proxy/conffiles
/etc/config/dnscrypt-proxy
/etc/config/public-resolvers
endef

define Package/luci-app-dnscrypt-proxy/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DATA) ./files/luci/controller/dnscrypt-proxy.lua $(1)/usr/lib/lua/luci/controller/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/dnscrypt-proxy.*.lmo $(1)/usr/lib/lua/luci/i18n/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/tools
	$(INSTALL_DATA) ./files/luci/tools/*.lua $(1)/usr/lib/lua/luci/tools/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi/dnscrypt-proxy
	$(INSTALL_DATA) ./files/luci/model/cbi/dnscrypt-proxy/*.lua $(1)/usr/lib/lua/luci/model/cbi/dnscrypt-proxy/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/view/dnscrypt-proxy
	$(INSTALL_DATA) ./files/luci/view/dnscrypt-proxy/*.htm $(1)/usr/lib/lua/luci/view/dnscrypt-proxy/
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/root/etc/uci-defaults/dnscrypt-proxy $(1)/etc/uci-defaults/
	$(INSTALL_DIR) $(1)/usr/share/dnscrypt-proxy
	$(INSTALL_BIN) ./files/uci_handle_exec.sh $(1)/usr/share/dnscrypt-proxy/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/dnscrypt-proxy.config $(1)/etc/config/dnscrypt-proxy
	$(INSTALL_DATA) ./files/public-resolvers.config $(1)/etc/config/public-resolvers
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/dnscrypt-proxy.init $(1)/etc/init.d/dnscrypt-proxy
endef

define Package/luci-app-dnscrypt-proxy/postinst
#!/bin/sh

if [ -z "$${IPKG_INSTROOT}" ]; then
	( . /etc/uci-defaults/dnscrypt-proxy ) && rm -f /etc/uci-defaults/dnscrypt-proxy
	chmod 755 /etc/init.d/dnscrypt-proxy >/dev/null 2>&1
	#/etc/init.d/dnscrypt-proxy enable >/dev/null 2>&1

	uci -q batch <<-EOF >/dev/null
		delete firewall.dnscrypt-proxy
		set firewall.dnscrypt-proxy=include
		set firewall.dnscrypt-proxy.type=script
		set firewall.dnscrypt-proxy.path=/var/etc/dnscrypt-proxy.include
		set firewall.dnscrypt-proxy.reload=0
		commit firewall
EOF
fi
exit 0
endef

define Package/luci-app-dnscrypt-proxy/prerm
#!/bin/sh
# check if we are on real system
if [ -z "$${IPKG_INSTROOT}" ]; then
    echo "Removing rc.d symlink for dnscrypt-proxy"
     /etc/init.d/dnscrypt-proxy disable
     /etc/init.d/dnscrypt-proxy stop
    echo "Removing firewall rule for dnscrypt-proxy"
	  uci -q batch <<-EOF >/dev/null
		delete firewall.dnscrypt-proxy
		commit firewall
EOF
fi
exit 0
endef

$(eval $(call BuildPackage,luci-app-dnscrypt-proxy))
