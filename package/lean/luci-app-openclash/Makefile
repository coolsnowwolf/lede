include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-openclash
PKG_VERSION:=0.39.8
PKG_RELEASE:=beta
PKG_MAINTAINER:=vernesong <https://github.com/vernesong/OpenClash>

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=LuCI support for clash
	PKGARCH:=all
	DEPENDS:=+iptables +dnsmasq-full +coreutils +coreutils-nohup +bash +curl +jsonfilter +ca-certificates +ipset +ip-full +iptables-mod-tproxy
	MAINTAINER:=vernesong
endef

define Package/$(PKG_NAME)/description
    A LuCI support for clash
endef

define Package/$(PKG_NAME)/postinst
endef

define Build/Prepare
	$(CP) $(CURDIR)/files $(PKG_BUILD_DIR)
	$(foreach po,$(wildcard ${CURDIR}/i18n/zh-cn/*.po), \
		po2lmo $(po) $(PKG_BUILD_DIR)/$(patsubst %.po,%.lmo,$(notdir $(po)));)
	chmod 0755 $(PKG_BUILD_DIR)/files/etc/init.d/openclash
	chmod -R 0755 $(PKG_BUILD_DIR)/files/usr/share/openclash/
	mkdir -p $(PKG_BUILD_DIR)/files/etc/openclash/config >/dev/null 2>&1
	mkdir -p $(PKG_BUILD_DIR)/files/etc/openclash/rule_provider >/dev/null 2>&1
	mkdir -p $(PKG_BUILD_DIR)/files/etc/openclash/backup >/dev/null 2>&1
	mkdir -p $(PKG_BUILD_DIR)/files/etc/openclash/core >/dev/null 2>&1
	mkdir -p $(PKG_BUILD_DIR)/files/usr/share/openclash/backup >/dev/null 2>&1
	cp "$(PKG_BUILD_DIR)/files/etc/config/openclash" "$(PKG_BUILD_DIR)/files/usr/share/openclash/backup/openclash" >/dev/null 2>&1
	cp "$(PKG_BUILD_DIR)/files/etc/openclash/custom/openclash_custom_rules.list" "$(PKG_BUILD_DIR)/files/usr/share/openclash/backup/openclash_custom_rules.list" >/dev/null 2>&1
	cp "$(PKG_BUILD_DIR)/files/etc/openclash/custom/openclash_custom_rules_2.list" "$(PKG_BUILD_DIR)/files/usr/share/openclash/backup/openclash_custom_rules_2.list" >/dev/null 2>&1
	cp "$(PKG_BUILD_DIR)/files/etc/openclash/custom/openclash_custom_hosts.list" "$(PKG_BUILD_DIR)/files/usr/share/openclash/backup/openclash_custom_hosts.list" >/dev/null 2>&1
	cp "$(PKG_BUILD_DIR)/files/etc/openclash/custom/openclash_custom_fake_filter.list" "$(PKG_BUILD_DIR)/files/usr/share/openclash/backup/openclash_custom_fake_filter.list" >/dev/null 2>&1
	cp "$(PKG_BUILD_DIR)/files/etc/openclash/custom/openclash_custom_domain_dns.list" "$(PKG_BUILD_DIR)/files/usr/share/openclash/backup/openclash_custom_domain_dns.list" >/dev/null 2>&1
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/preinst
#!/bin/sh
if [ -f "/etc/config/openclash" ]; then
	cp "/etc/config/openclash" "/tmp/openclash.bak" >/dev/null 2>&1
	cp "/etc/config/openclash_custom_rules.list" "/tmp/openclash_custom_rules.list.bak" >/dev/null 2>&1
	cp "/etc/config/openclash_custom_rules_2.list" "/tmp/openclash_custom_rules_2.list.bak" >/dev/null 2>&1
	cp "/etc/config/openclash_custom_hosts.list" "/tmp/openclash_custom_hosts.list.bak" >/dev/null 2>&1
	cp "/etc/config/openclash_custom_fake_filter.list" "/tmp/openclash_custom_fake_filter.list.bak" >/dev/null 2>&1
	cp -r "/etc/openclash/history" "/tmp/openclash_history" >/dev/null 2>&1
fi
if [ -f "/etc/openclash/custom/openclash_custom_rules.list" ]; then
	cp "/etc/openclash/custom/openclash_custom_rules.list" "/tmp/openclash_custom_rules.list.bak" >/dev/null 2>&1
	cp "/etc/openclash/custom/openclash_custom_rules_2.list" "/tmp/openclash_custom_rules_2.list.bak" >/dev/null 2>&1
	cp "/etc/openclash/custom/openclash_custom_hosts.list" "/tmp/openclash_custom_hosts.list.bak" >/dev/null 2>&1
	cp "/etc/openclash/custom/openclash_custom_fake_filter.list" "/tmp/openclash_custom_fake_filter.list.bak" >/dev/null 2>&1
	cp "/etc/openclash/custom/openclash_custom_domain_dns.list" "/tmp/openclash_custom_domain_dns.list.bak" >/dev/null 2>&1
fi
endef

define Package/$(PKG_NAME)/postinst
#!/bin/sh
uci delete ucitrack.@openclash[-1] >/dev/null 2>&1
uci add ucitrack openclash >/dev/null 2>&1
uci set ucitrack.@openclash[-1].init=openclash >/dev/null 2>&1
uci commit ucitrack >/dev/null 2>&1
uci delete firewall.openclash >/dev/null 2>&1
uci set firewall.openclash=include >/dev/null 2>&1
uci set firewall.openclash.type=script >/dev/null 2>&1
uci set firewall.openclash.path=/var/etc/openclash.include >/dev/null 2>&1
uci set firewall.openclash.reload=1 >/dev/null 2>&1
uci commit firewall >/dev/null 2>&1
mkdir -p /etc/openclash/config >/dev/null 2>&1
mkdir -p /etc/openclash/proxy_provider >/dev/null 2>&1
mkdir -p /etc/openclash/backup >/dev/null 2>&1
mkdir -p /etc/openclash/core >/dev/null 2>&1
mkdir -p /etc/openclash/history >/dev/null 2>&1
mkdir -p /usr/share/openclash/backup >/dev/null 2>&1

cp "/etc/config/openclash" "/usr/share/openclash/backup/openclash" >/dev/null 2>&1
cp "/etc/openclash/custom/openclash_custom_rules.list" "/usr/share/openclash/backup/openclash_custom_rules.list" >/dev/null 2>&1
cp "/etc/openclash/custom/openclash_custom_rules_2.list" "/usr/share/openclash/backup/openclash_custom_rules_2.list" >/dev/null 2>&1
cp "/etc/openclash/custom/openclash_custom_hosts.list" "/usr/share/openclash/backup/openclash_custom_hosts.list" >/dev/null 2>&1
cp "/etc/openclash/custom/openclash_custom_fake_filter.list" "/usr/share/openclash/backup/openclash_custom_fake_filter.list" >/dev/null 2>&1
cp "/etc/openclash/custom/openclash_custom_domain_dns.list" "/usr/share/openclash/backup/openclash_custom_domain_dns.list" >/dev/null 2>&1

if [ -f "/tmp/openclash.bak" ]; then
	mv "/tmp/openclash.bak" "/etc/config/openclash" >/dev/null 2>&1
	mv "/tmp/openclash_custom_rules.list.bak" "/etc/openclash/custom/openclash_custom_rules.list" >/dev/null 2>&1
	mv "/tmp/openclash_custom_rules_2.list.bak" "/etc/openclash/custom/openclash_custom_rules_2.list" >/dev/null 2>&1
	mv "/tmp/openclash_custom_hosts.list.bak" "/etc/openclash/custom/openclash_custom_hosts.list" >/dev/null 2>&1
	mv "/tmp/openclash_custom_fake_filter.list.bak" "/etc/openclash/custom/openclash_custom_fake_filter.list" >/dev/null 2>&1
	mv "/tmp/openclash_custom_domain_dns.list.bak" "/etc/openclash/custom/openclash_custom_domain_dns.list" >/dev/null 2>&1
	if [ -d "/tmp/openclash_history/" ]; then
		cp -a "/tmp/openclash_history/." "/etc/openclash/history" >/dev/null 2>&1
		rm -rf "/tmp/openclash_history" >/dev/null 2>&1
	fi
fi
if [ -f "/tmp/config.yaml" ]; then
	mv "/tmp/config.yaml" "/etc/openclash/config/config.yaml" >/dev/null 2>&1
elif [ -f "/tmp/config.yml" ]; then
	mv "/tmp/config.yml" "/etc/openclash/config/config.yaml" >/dev/null 2>&1
elif [ -d "/tmp/openclash_config/" ]; then
	rm -rf "/etc/openclash/config" >/dev/null 2>&1
	mv "/tmp/openclash_config" "/etc/openclash/config" >/dev/null 2>&1
fi
if [ -d "/tmp/openclash_proxy_provider/" ]; then
	rm -rf "/etc/openclash/proxy_provider" >/dev/null 2>&1
	mv "/tmp/openclash_proxy_provider" "/etc/openclash/proxy_provider" >/dev/null 2>&1
fi
if [ -d "/tmp/openclash_rule_provider/" ]; then
	cp -a "/tmp/openclash_rule_provider/." "/etc/openclash/rule_provider" >/dev/null 2>&1
	rm -rf "/tmp/openclash_rule_provider" >/dev/null 2>&1
fi
if [ -d "/tmp/openclash_game_rules/" ]; then
	rm -rf "/etc/openclash/game_rules" >/dev/null 2>&1
	mv "/tmp/openclash_game_rules" "/etc/openclash/game_rules" >/dev/null 2>&1
fi
if [ -f "/tmp/clash" ]; then
	mv "/tmp/clash" "/etc/openclash/core/clash" >/dev/null 2>&1
fi
if [ -f "/etc/openclash/clash" ]; then
	mv "/etc/openclash/clash" "/etc/openclash/core/clash" >/dev/null 2>&1
fi
if [ -d "/tmp/openclash_core/" ]; then
	rm -rf "/etc/openclash/core" >/dev/null 2>&1
	mv "/tmp/openclash_core" "/etc/openclash/core" >/dev/null 2>&1
fi
mkdir -p /lib/upgrade/keep.d >/dev/null 2>&1
cat > "/lib/upgrade/keep.d/luci-app-openclash" <<-EOF
/etc/openclash/core/
/etc/openclash/custom/
/etc/openclash/config/
/etc/openclash/game_rules/
/etc/openclash/proxy_provider/
/etc/openclash/rule_provider/
EOF
uci set openclash.config.enable=0 >/dev/null 2>&1
uci commit openclash >/dev/null 2>&1
chmod 0755 /etc/init.d/openclash >/dev/null 2>&1
chmod -R 0755 /usr/share/openclash/ >/dev/null 2>&1
rm -rf /tmp/luci*
endef

define Package/$(PKG_NAME)/prerm
#!/bin/sh
	uci set openclash.config.enable=0 >/dev/null 2>&1
	uci commit openclash >/dev/null 2>&1
	mv "/etc/openclash/core" "/tmp/openclash_core" >/dev/null 2>&1
	mv "/etc/openclash/config.yaml" "/tmp/config.yaml" >/dev/null 2>&1
	mv "/etc/openclash/config" "/tmp/openclash_config" >/dev/null 2>&1
	mv "/etc/openclash/proxy_provider" "/tmp/openclash_proxy_provider" >/dev/null 2>&1
	mv "/etc/openclash/rule_provider" "/tmp/openclash_rule_provider" >/dev/null 2>&1
	mv "/etc/openclash/game_rules" "/tmp/openclash_game_rules" >/dev/null 2>&1
	mv "/etc/openclash/history" "/tmp/openclash_history" >/dev/null 2>&1
	cp "/etc/config/openclash" "/tmp/openclash.bak" >/dev/null 2>&1
	cp "/etc/config/openclash_custom_rules.list" "/tmp/openclash_custom_rules.list.bak" >/dev/null 2>&1
	cp "/etc/config/openclash_custom_hosts.list" "/tmp/openclash_custom_hosts.list.bak" >/dev/null 2>&1
	cp "/etc/config/openclash_custom_fake_filter.list" "/tmp/openclash_custom_fake_filter.list.bak" >/dev/null 2>&1
	cp "/etc/openclash/custom/openclash_custom_rules.list" "/tmp/openclash_custom_rules.list.bak" >/dev/null 2>&1
	cp "/etc/openclash/custom/openclash_custom_rules_2.list" "/tmp/openclash_custom_rules_2.list.bak" >/dev/null 2>&1
	cp "/etc/openclash/custom/openclash_custom_hosts.list" "/tmp/openclash_custom_hosts.list.bak" >/dev/null 2>&1
	cp "/etc/openclash/custom/openclash_custom_fake_filter.list" "/tmp/openclash_custom_fake_filter.list.bak" >/dev/null 2>&1
	cp "/etc/openclash/custom/openclash_custom_domain_dns.list" "/tmp/openclash_custom_domain_dns.list.bak" >/dev/null 2>&1
endef

define Package/$(PKG_NAME)/postrm
#!/bin/sh
if [ -f "/tmp/openclash_update.sh" ]; then
	rm -rf /etc/openclash/config >/dev/null 2>&1
	rm -rf /etc/openclash/backup >/dev/null 2>&1
	rm -rf /etc/openclash/proxy_provider >/dev/null 2>&1
	rm -rf /etc/openclash/rule_provider >/dev/null 2>&1
	rm -rf /etc/openclash/game_rules >/dev/null 2>&1
	rm -rf /etc/openclash/history >/dev/null 2>&1
else
	rm -rf /etc/openclash >/dev/null 2>&1
fi
	rm -rf /lib/upgrade/keep.d/luci-app-openclash >/dev/null 2>&1
	rm -rf /tmp/openclash.log >/dev/null 2>&1
	rm -rf /tmp/openclash_start.log >/dev/null 2>&1
	rm -rf /tmp/Proxy_Group >/dev/null 2>&1
	rm -rf /tmp/rules_name >/dev/null 2>&1
	rm -rf /tmp/openclash_last_version >/dev/null 2>&1
	rm -rf /tmp/clash_last_version >/dev/null 2>&1
	rm -rf /etc/openclash/dnsmasq_fake_block.conf >/dev/null 2>&1
	rm -rf /etc/openclash/clash >/dev/null 2>&1
	uci delete firewall.openclash >/dev/null 2>&1
	uci commit firewall >/dev/null 2>&1
	uci delete ucitrack.@openclash[-1] >/dev/null 2>&1
	uci commit ucitrack >/dev/null 2>&1
	rm -rf /tmp/luci*
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/*.*.lmo $(1)/usr/lib/lua/luci/i18n/
	$(CP) $(PKG_BUILD_DIR)/files/* $(1)/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
