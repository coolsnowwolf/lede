include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-vssr
PKG_VERSION:=1.11
PKG_RELEASE:=20200719

PKG_CONFIG_DEPENDS:= CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_V2ray \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Server \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Socks

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)/config
	
config PACKAGE_$(PKG_NAME)_INCLUDE_V2ray
	bool "Include V2ray"
	default y

config PACKAGE_$(PKG_NAME)_INCLUDE_Trojan
	bool "Include Trojan"
	default y

config PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Server
	bool "Include ShadowsocksR Server"
	default n
	
config PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Socks
	bool "Include ShadowsocksR Socks and Tunnel"
	default y
endef

define Package/luci-app-vssr
 	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=A New SS/SSR/V2Ray/Trojan LuCI interface
	PKGARCH:=all
	DEPENDS:=+shadowsocksr-libev-alt +ipset +ip-full +iptables-mod-tproxy +dnsmasq-full +coreutils +coreutils-base64 +bash +pdnsd-alt +wget +luasocket +jshn +lua-cjson +coreutils-nohup +lua-maxminddb \
            +PACKAGE_$(PKG_NAME)_INCLUDE_V2ray:v2ray \
			+PACKAGE_$(PKG_NAME)_INCLUDE_Trojan:trojan \
			+PACKAGE_$(PKG_NAME)_INCLUDE_Trojan:ipt2socks \
            +PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Server:shadowsocksr-libev-server \
            +PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Socks:shadowsocksr-libev-ssr-local
endef

define Build/Prepare
endef

define Build/Compile
endef

define Package/luci-app-vssr/conffiles
	/etc/ssr_ip
	/etc/dnsmasq.ssr/gfw_list.conf
	/etc/china_ssr.txt
	/etc/dnsmasq.ssr/gfw_list.conf
	/etc/dnsmasq.ssr/gfw_base.conf
	/etc/dnsmasq.oversea/oversea_list.conf
endef

define Package/luci-app-vssr/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci
	cp -pR ./luasrc/* $(1)/usr/lib/lua/luci
	$(INSTALL_DIR) $(1)/
	cp -pR ./root/* $(1)/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	po2lmo ./po/zh-cn/vssr.po $(1)/usr/lib/lua/luci/i18n/vssr.zh-cn.lmo
endef

define Package/luci-app-vssr/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	( . /etc/uci-defaults/luci-vssr ) && rm -f /etc/uci-defaults/luci-vssr
	rm -rf /tmp/luci-indexcache
	rm -rf /tmp/luci-modulecache/*
	chmod 755 /etc/init.d/vssr >/dev/null 2>&1
	/etc/init.d/vssr enable >/dev/null 2>&1
fi
exit 0
endef

define Package/luci-app-vssr/prerm
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
     /etc/init.d/vssr disable
     /etc/init.d/vssr stop
fi
exit 0
endef

$(eval $(call BuildPackage,luci-app-vssr))
