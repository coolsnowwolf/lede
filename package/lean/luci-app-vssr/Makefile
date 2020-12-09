include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-vssr
PKG_VERSION:=1.20
PKG_RELEASE:=20201209

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)/config
config PACKAGE_$(PKG_NAME)_INCLUDE_V2ray_plugin
	bool "Include Shadowsocks V2ray Plugin"
	default y if i386||x86_64||arm||aarch64

config PACKAGE_$(PKG_NAME)_INCLUDE_V2ray
	bool "Include V2ray"
	default y if i386||x86_64||arm||aarch64

config PACKAGE_$(PKG_NAME)_INCLUDE_Xray
	bool "Include Xray"
	default n

config PACKAGE_$(PKG_NAME)_INCLUDE_Trojan
	bool "Include Trojan"
	default y if i386||x86_64||arm||aarch64
	
config PACKAGE_$(PKG_NAME)_INCLUDE_Kcptun
	bool "Include Kcptun"
	default n

config PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Server
	bool "Include ShadowsocksR Server"
	default y if i386||x86_64||arm||aarch64
endef

PKG_CONFIG_DEPENDS:= \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_V2ray_plugin \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_V2ray \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_Xray \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_Trojan \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_Kcptun \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Server

define Package/luci-app-vssr
 	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=A New SS/SSR/V2Ray/Trojan LuCI interface
	PKGARCH:=all
	DEPENDS:=+shadowsocksr-libev-alt +ipset +ip-full +iptables-mod-tproxy +dnsmasq-full +coreutils +coreutils-base64 +bash +pdnsd-alt +wget +luasocket +lua-maxminddb \
			+shadowsocks-libev-ss-local +shadowsocksr-libev-ssr-local +shadowsocks-libev-ss-redir +simple-obfs \
			+PACKAGE_$(PKG_NAME)_INCLUDE_V2ray_plugin:v2ray-plugin \
			+PACKAGE_$(PKG_NAME)_INCLUDE_V2ray:v2ray \
			+PACKAGE_$(PKG_NAME)_INCLUDE_Xray:xray \
			+PACKAGE_$(PKG_NAME)_INCLUDE_Trojan:trojan \
			+PACKAGE_$(PKG_NAME)_INCLUDE_Trojan:ipt2socks \
			+PACKAGE_$(PKG_NAME)_INCLUDE_Kcptun:kcptun-client \
			+PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Server:shadowsocksr-libev-server
endef

define Build/Prepare
endef

define Build/Compile
endef

define Package/luci-app-vssr/conffiles
	/etc/vssr/
	/etc/config/vssr
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
