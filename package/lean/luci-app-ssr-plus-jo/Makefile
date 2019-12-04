include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-ssr-plus-jo
PKG_VERSION:=1.35
PKG_RELEASE:=8
PKG_CONFIG_DEPENDS:= CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_V2ray \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_Kcptun:kcptun \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Server \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks_Server \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Socks \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks_Socks \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_dnscrypt_proxy \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_dnsforwarder \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_ChinaDNS \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_haproxy \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_privoxy \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_simple-obfs\
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_simple-obfs-server\
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_GoQuiet-client\
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_GoQuiet-server\
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_v2ray-plugin

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)/config
config PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks
	bool "Include Shadowsocks New Version"
	default y
	
config PACKAGE_$(PKG_NAME)_INCLUDE_V2ray
	bool "Include V2ray"
	default y
	
config PACKAGE_$(PKG_NAME)_INCLUDE_Kcptun
	bool "Include Kcptun"
	default y
	
config PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Server
	bool "Include ShadowsocksR Server"
	default y

config PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks_Server
	bool "Include Shadowsocks Server"
	default n
	
config PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Socks
	bool "Include ShadowsocksR Socks and Tunnel"
	default y

config PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks_Socks
	bool "Include Shadowsocks Socks and Tunnel"
	default n

config PACKAGE_$(PKG_NAME)_INCLUDE_dnscrypt_proxy
	bool "dnscrypt-proxy-full"
	default n

config PACKAGE_$(PKG_NAME)_INCLUDE_dnsforwarder
	bool "dnsforwarder"
	default n

config PACKAGE_$(PKG_NAME)_INCLUDE_ChinaDNS
	bool "chiandns"
	default n

config PACKAGE_$(PKG_NAME)_INCLUDE_haproxy
	bool "haproxy"
	default n

config PACKAGE_$(PKG_NAME)_INCLUDE_privoxy
	bool "privoxy http local"
	default n

config PACKAGE_$(PKG_NAME)_INCLUDE_simple-obfs
	bool "simple-obfsl"
	default n
	
config PACKAGE_$(PKG_NAME)_INCLUDE_simple-obfs-server
	bool "simple-obfs-server"
	default n
	
config PACKAGE_$(PKG_NAME)_INCLUDE_GoQuiet-client
	bool "GoQuiet-client"
	default n
	
config PACKAGE_$(PKG_NAME)_INCLUDE_GoQuiet-server
	bool "GoQuiet-server"
	default n

config PACKAGE_$(PKG_NAME)_INCLUDE_v2ray-plugin
	bool "v2ray-plugin"
	default n
endef

define Package/luci-app-ssr-plus-jo
 	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=SS/SSR/V2Ray LuCI interface
	PKGARCH:=all
	DEPENDS:=+shadowsocksr-libev-alt    +ipset +ip-full +iptables-mod-tproxy +dnsmasq-full +coreutils +coreutils-base64 +bash pdnsd-alt +wget \
            +PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks:shadowsocks-libev-ss-redir \
            +PACKAGE_$(PKG_NAME)_INCLUDE_V2ray:v2ray \
            +PACKAGE_$(PKG_NAME)_INCLUDE_Kcptun:kcptun-client \
            +PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Server:shadowsocksr-libev-server \
            +PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks_Server:shadowsocks-libev-ss-server \
            +PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Socks:shadowsocksr-libev-ssr-local \
            +PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks_Socks:shadowsocks-libev-ss-local \
            +PACKAGE_$(PKG_NAME)_INCLUDE_dnscrypt_proxy:dnscrypt-proxy-full \
            +PACKAGE_$(PKG_NAME)_INCLUDE_dnsforwarder:dnsforwarder \
            +PACKAGE_$(PKG_NAME)_INCLUDE_ChinaDNS:ChinaDNS \
            +PACKAGE_$(PKG_NAME)_INCLUDE_haproxy:haproxy \
            +PACKAGE_$(PKG_NAME)_INCLUDE_privoxy:privoxy \
            +PACKAGE_$(PKG_NAME)_INCLUDE_simple-obfs:simple-obfs \
            +PACKAGE_$(PKG_NAME)_INCLUDE_simple-obfs-server:simple-obfs-server \
            +PACKAGE_$(PKG_NAME)_INCLUDE_GoQuiet-client:gq-client \
			+PACKAGE_$(PKG_NAME)_INCLUDE_GoQuiet-server:gq-server \
            +PACKAGE_$(PKG_NAME)_INCLUDE_v2ray-plugin:v2ray-plugin
endef

define Build/Prepare
endef

define Build/Compile
endef

define Package/luci-app-ssr-plus-jo/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci
	cp -pR ./luasrc/* $(1)/usr/lib/lua/luci
	$(INSTALL_DIR) $(1)/
	cp -pR ./root/* $(1)/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	cp -pR ./po/zh-cn/ssr-plus.zh-cn.lmo $(1)/usr/lib/lua/luci/i18n/ssr-plus.zh-cn.lmo
endef

define Package/luci-app-ssr-plus-jo/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	( . /etc/uci-defaults/luci-ssr-plus ) && rm -f /etc/uci-defaults/luci-ssr-plus
	rm -f /tmp/luci-indexcache
	chmod 755 /etc/init.d/shadowsocksr >/dev/null 2>&1
	/etc/init.d/shadowsocksr enable >/dev/null 2>&1
fi
exit 0
endef

define Package/luci-app-ssr-plus-jo/prerm
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
     /etc/init.d/shadowsocksr disable
     /etc/init.d/shadowsocksr stop
fi
exit 0
endef

$(eval $(call BuildPackage,luci-app-ssr-plus-jo))
