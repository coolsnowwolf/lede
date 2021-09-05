include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-openclash
PKG_VERSION:=0.43.04
PKG_RELEASE:=beta
PKG_MAINTAINER:=vernesong <https://github.com/vernesong/OpenClash>

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)/config
	select PACKAGE_libcap-bin
endef

define Package/$(PKG_NAME)
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=LuCI support for clash
	PKGARCH:=all
	DEPENDS:=+iptables +dnsmasq-full +coreutils +coreutils-nohup +bash +curl +ca-certificates +ipset +ip-full +iptables-mod-tproxy +iptables-mod-extra +libcap +ruby +ruby-yaml
	MAINTAINER:=vernesong
endef

define Package/$(PKG_NAME)/description
    A LuCI support for clash
endef

define Build/Prepare
	$(CP) $(CURDIR)/root $(PKG_BUILD_DIR)
	$(CP) $(CURDIR)/luasrc $(PKG_BUILD_DIR)
	$(foreach po,$(wildcard ${CURDIR}/po/zh-cn/*.po), \
		po2lmo $(po) $(PKG_BUILD_DIR)/$(patsubst %.po,%.lmo,$(notdir $(po)));)
	chmod 0755 $(PKG_BUILD_DIR)/root/etc/init.d/openclash
	chmod -R 0755 $(PKG_BUILD_DIR)/root/usr/share/openclash/
	mkdir -p $(PKG_BUILD_DIR)/root/etc/openclash/config
	mkdir -p $(PKG_BUILD_DIR)/root/etc/openclash/rule_provider
	mkdir -p $(PKG_BUILD_DIR)/root/etc/openclash/backup
	mkdir -p $(PKG_BUILD_DIR)/root/etc/openclash/core
	mkdir -p $(PKG_BUILD_DIR)/root/usr/share/openclash/backup
	cp -f "$(PKG_BUILD_DIR)/root/etc/config/openclash" "$(PKG_BUILD_DIR)/root/usr/share/openclash/backup/openclash" >/dev/null 2>&1
	cp -f "$(PKG_BUILD_DIR)/root/etc/openclash/custom/openclash_custom_rules.list" "$(PKG_BUILD_DIR)/root/usr/share/openclash/backup/openclash_custom_rules.list" >/dev/null 2>&1
	cp -f "$(PKG_BUILD_DIR)/root/etc/openclash/custom/openclash_custom_rules_2.list" "$(PKG_BUILD_DIR)/root/usr/share/openclash/backup/openclash_custom_rules_2.list" >/dev/null 2>&1
	cp -f "$(PKG_BUILD_DIR)/root/etc/openclash/custom/openclash_custom_hosts.list" "$(PKG_BUILD_DIR)/root/usr/share/openclash/backup/openclash_custom_hosts.list" >/dev/null 2>&1
	cp -f "$(PKG_BUILD_DIR)/root/etc/openclash/custom/openclash_custom_fake_filter.list" "$(PKG_BUILD_DIR)/root/usr/share/openclash/backup/openclash_custom_fake_filter.list" >/dev/null 2>&1
	cp -f "$(PKG_BUILD_DIR)/root/etc/openclash/custom/openclash_custom_domain_dns.list" "$(PKG_BUILD_DIR)/root/usr/share/openclash/backup/openclash_custom_domain_dns.list" >/dev/null 2>&1
	cp -f "$(PKG_BUILD_DIR)/root/etc/openclash/custom/openclash_custom_domain_dns_policy.list" "$(PKG_BUILD_DIR)/root/usr/share/openclash/backup/openclash_custom_domain_dns_policy.list" >/dev/null 2>&1
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/conffiles
endef

define Package/$(PKG_NAME)/preinst
#!/bin/sh
if [ -f "/etc/config/openclash" ]; then
	cp -f "/etc/config/openclash" "/tmp/openclash.bak" >/dev/null 2>&1
	cp -rf "/etc/openclash" "/tmp/openclash" >/dev/null 2>&1
fi
endef

define Package/$(PKG_NAME)/postinst
endef

define Package/$(PKG_NAME)/prerm
#!/bin/sh
	uci -q set openclash.config.enable=0
	uci -q commit openclash
	cp -f "/etc/config/openclash" "/tmp/openclash.bak" >/dev/null 2>&1
	cp -rf "/etc/openclash" "/tmp/openclash" >/dev/null 2>&1
endef

define Package/$(PKG_NAME)/postrm
#!/bin/sh
	rm -rf /etc/openclash
	rm -rf /tmp/openclash.log
	rm -rf /tmp/openclash_start.log
	rm -rf /tmp/openclash_last_version
	rm -rf /tmp/Proxy_Group
	rm -rf /tmp/rules_name
	rm -rf /tmp/rule_providers_name
	rm -rf /tmp/clash_last_version
	rm -rf /usr/share/openclash/backup
	rm -rf /tmp/openclash_fake_filter.list
	rm -rf /tmp/openclash_servers_fake_filter.conf
	uci -q delete firewall.openclash
	uci -q commit firewall
	uci -q delete ucitrack.@openclash[-1]
	uci -q commit ucitrack
	rm -rf /tmp/luci-*
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/*.*.lmo $(1)/usr/lib/lua/luci/i18n/
	$(CP) $(PKG_BUILD_DIR)/root/* $(1)/
	$(CP) $(PKG_BUILD_DIR)/luasrc/* $(1)/usr/lib/lua/luci/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
