include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-ssr-plus
PKG_VERSION:=8
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

PKG_CONFIG_DEPENDS:=CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Server \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Socks

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)/config
	
config PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR
	bool "Include ShadowsocksR Client"
	default y if x86_64
	
config PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Server
	bool "Include ShadowsocksR Server"
	default y if x86_64
	
config PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Socks
	bool "Include ShadowsocksR Socks and Tunnel"
	default y if x86_64
endef

define Package/$(PKG_NAME)
 	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=SS/SSR/V2Ray/Trojan/Trojan-Go LuCI interface
	PKGARCH:=all
	DEPENDS:=+tcping +ipset +ip-full +iptables-mod-tproxy +dnsmasq-full +coreutils +coreutils-base64 +pdnsd-alt +wget \
            +PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR:shadowsocksr-libev-alt \
            +PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Server:shadowsocksr-libev-server \
            +PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Socks:shadowsocksr-libev-ssr-local
endef

define Build/Prepare
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/conffiles
/etc/config/shadowsocksr
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc/ssr
	$(INSTALL_DATA) ./root/etc/ssr/* $(1)/etc/ssr/

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./root/etc/config/shadowsocksr $(1)/etc/config/shadowsocksr

	$(INSTALL_DIR) $(1)/etc/dnsmasq.oversea
	$(INSTALL_DATA) ./root/etc/dnsmasq.oversea/* $(1)/etc/dnsmasq.oversea/

	$(INSTALL_DIR) $(1)/etc/dnsmasq.ssr
	$(INSTALL_DATA) ./root/etc/dnsmasq.ssr/* $(1)/etc/dnsmasq.ssr/

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./root/etc/init.d/* $(1)/etc/init.d/

	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./root/etc/uci-defaults/* $(1)/etc/uci-defaults/

	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./root/usr/bin/* $(1)/usr/bin/

	$(INSTALL_DIR) $(1)/usr/share/shadowsocksr
	$(INSTALL_BIN) ./root/usr/share/shadowsocksr/*.sh $(1)/usr/share/shadowsocksr/
	$(INSTALL_DATA) ./root/usr/share/shadowsocksr/*.lua $(1)/usr/share/shadowsocksr/

	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DATA) ./luasrc/controller/*.lua $(1)/usr/lib/lua/luci/controller/

	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi/shadowsocksr
	$(INSTALL_DATA) ./luasrc/model/cbi/shadowsocksr/*.lua $(1)/usr/lib/lua/luci/model/cbi/shadowsocksr/

	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/view/shadowsocksr
	$(INSTALL_DATA) ./luasrc/view/shadowsocksr/* $(1)/usr/lib/lua/luci/view/shadowsocksr/

	$(INSTALL_DIR) $(1)/usr/share/rpcd/acl.d
	$(INSTALL_DATA) ./root/usr/share/rpcd/acl.d/luci-app-ssr-plus.json $(1)/usr/share/rpcd/acl.d/

	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	po2lmo ./po/zh-cn/ssr-plus.po $(1)/usr/lib/lua/luci/i18n/ssr-plus.zh-cn.lmo
endef

define Package/$(PKG_NAME)/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	( . /etc/uci-defaults/luci-ssr-plus ) && rm -f /etc/uci-defaults/luci-ssr-plus
	rm -f /tmp/luci-indexcache
	chmod 755 /etc/init.d/shadowsocksr >/dev/null 2>&1
	/etc/init.d/shadowsocksr enable >/dev/null 2>&1
	uci -q batch <<-EOF >/dev/null
		delete firewall.shadowsocksr
		set firewall.shadowsocksr=include
		set firewall.shadowsocksr.type=script
		set firewall.shadowsocksr.path=/var/etc/shadowsocksr.include
		set firewall.shadowsocksr.reload=1
		commit firewall
EOF
fi
exit 0
endef

define Package/luci-app-ssr-plus/prerm
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
     /etc/init.d/shadowsocksr disable
     /etc/init.d/shadowsocksr stop
    echo "Removing firewall rule for shadowsocksr"
	  uci -q batch <<-EOF >/dev/null
		delete firewall.shadowsocksr
		commit firewall
EOF
fi
exit 0
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
