include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-openclash
PKG_VERSION:=0.40.7
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

define Build/Prepare
	$(CP) $(CURDIR)/root $(PKG_BUILD_DIR)
	$(CP) $(CURDIR)/luasrc $(PKG_BUILD_DIR)
	$(foreach po,$(wildcard ${CURDIR}/po/zh-cn/*.po), \
		po2lmo $(po) $(PKG_BUILD_DIR)/$(patsubst %.po,%.lmo,$(notdir $(po)));)
	chmod 0755 $(PKG_BUILD_DIR)/root/etc/init.d/openclash
	chmod -R 0755 $(PKG_BUILD_DIR)/root/usr/share/openclash/
	mkdir -p $(PKG_BUILD_DIR)/root/etc/openclash/config >/dev/null 2>&1
	mkdir -p $(PKG_BUILD_DIR)/root/etc/openclash/rule_provider >/dev/null 2>&1
	mkdir -p $(PKG_BUILD_DIR)/root/etc/openclash/backup >/dev/null 2>&1
	mkdir -p $(PKG_BUILD_DIR)/root/etc/openclash/core >/dev/null 2>&1
	mkdir -p $(PKG_BUILD_DIR)/root/usr/share/openclash/backup >/dev/null 2>&1
	cp "$(PKG_BUILD_DIR)/root/etc/config/openclash" "$(PKG_BUILD_DIR)/root/usr/share/openclash/backup/openclash" >/dev/null 2>&1
	cp "$(PKG_BUILD_DIR)/root/etc/openclash/custom/openclash_custom_rules.list" "$(PKG_BUILD_DIR)/root/usr/share/openclash/backup/openclash_custom_rules.list" >/dev/null 2>&1
	cp "$(PKG_BUILD_DIR)/root/etc/openclash/custom/openclash_custom_rules_2.list" "$(PKG_BUILD_DIR)/root/usr/share/openclash/backup/openclash_custom_rules_2.list" >/dev/null 2>&1
	cp "$(PKG_BUILD_DIR)/root/etc/openclash/custom/openclash_custom_hosts.list" "$(PKG_BUILD_DIR)/root/usr/share/openclash/backup/openclash_custom_hosts.list" >/dev/null 2>&1
	cp "$(PKG_BUILD_DIR)/root/etc/openclash/custom/openclash_custom_fake_filter.list" "$(PKG_BUILD_DIR)/root/usr/share/openclash/backup/openclash_custom_fake_filter.list" >/dev/null 2>&1
	cp "$(PKG_BUILD_DIR)/root/etc/openclash/custom/openclash_custom_domain_dns.list" "$(PKG_BUILD_DIR)/root/usr/share/openclash/backup/openclash_custom_domain_dns.list" >/dev/null 2>&1
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
	cp "/etc/config/openclash" "/tmp/openclash.bak" >/dev/null 2>&1
	cp -r "/etc/openclash" "/tmp/openclash" >/dev/null 2>&1
fi
endef

define Package/$(PKG_NAME)/postinst
endef

define Package/$(PKG_NAME)/prerm
#!/bin/sh
	uci set openclash.config.enable=0 >/dev/null 2>&1
	uci commit openclash >/dev/null 2>&1
	cp "/etc/config/openclash" "/tmp/openclash.bak" >/dev/null 2>&1
	cp -r "/etc/openclash" "/tmp/openclash" >/dev/null 2>&1
endef

define Package/$(PKG_NAME)/postrm
#!/bin/sh
	rm -rf /etc/openclash >/dev/null 2>&1
	rm -rf /tmp/openclash.log >/dev/null 2>&1
	rm -rf /tmp/openclash_start.log >/dev/null 2>&1
	rm -rf /tmp/openclash_last_version >/dev/null 2>&1
	rm -rf /tmp/Proxy_Group >/dev/null 2>&1
	rm -rf /tmp/rules_name >/dev/null 2>&1
	rm -rf /tmp/rule_providers_name >/dev/null 2>&1
	rm -rf /tmp/clash_last_version >/dev/null 2>&1
	rm -rf /usr/share/openclash/backup >/dev/null 2>&1
	uci delete firewall.openclash >/dev/null 2>&1
	uci commit firewall >/dev/null 2>&1
	uci delete ucitrack.@openclash[-1] >/dev/null 2>&1
	uci commit ucitrack >/dev/null 2>&1
	rm -rf /tmp/luci* >/dev/null 2>&1
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/*.*.lmo $(1)/usr/lib/lua/luci/i18n/
	$(CP) $(PKG_BUILD_DIR)/root/* $(1)/
	$(CP) $(PKG_BUILD_DIR)/luasrc/* $(1)/usr/lib/lua/luci/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
