include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-ssr-plus
PKG_VERSION:=1
PKG_RELEASE:=68

PKG_CONFIG_DEPENDS:= CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_V2ray \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_Kcptun:kcptun \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Server \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Socks

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)/config
config PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks
	bool "Include Shadowsocks New Version"
	default n
	
config PACKAGE_$(PKG_NAME)_INCLUDE_V2ray
	bool "Include V2ray"
	default n
	
config PACKAGE_$(PKG_NAME)_INCLUDE_Kcptun
	bool "Include Kcptun"
	default n
	
config PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Server
	bool "Include ShadowsocksR Server"
	default n
	
config PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Socks
	bool "Include ShadowsocksR Socks and Tunnel"
	default n
endef

define Package/luci-app-ssr-plus
 	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=SS/SSR/V2Ray LuCI interface
	PKGARCH:=all
	DEPENDS:=+shadowsocksr-libev-alt +ipset +ip-full +iptables-mod-tproxy +dnsmasq-full +coreutils +coreutils-base64 +bash +pdnsd-alt +wget \
            +PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks:shadowsocks-libev-ss-redir \
            +PACKAGE_$(PKG_NAME)_INCLUDE_V2ray:v2ray \
            +PACKAGE_$(PKG_NAME)_INCLUDE_Kcptun:kcptun \
            +PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Server:shadowsocksr-libev-server \
            +PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Socks:shadowsocksr-libev-ssr-local
endef

define Build/Prepare
endef

define Build/Compile
endef

define Package/luci-app-ssr-plus/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci
	cp -pR ./luasrc/* $(1)/usr/lib/lua/luci
	$(INSTALL_DIR) $(1)/
	cp -pR ./root/* $(1)/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	po2lmo ./po/zh-cn/ssr-plus.po $(1)/usr/lib/lua/luci/i18n/ssr-plus.zh-cn.lmo
endef

define Package/luci-app-ssr-plus/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	( . /etc/uci-defaults/luci-ssr-plus ) && rm -f /etc/uci-defaults/luci-ssr-plus
	rm -f /tmp/luci-indexcache
	chmod 755 /etc/init.d/shadowsocksr >/dev/null 2>&1
	/etc/init.d/shadowsocksr enable >/dev/null 2>&1
fi
exit 0
endef

define Package/luci-app-ssr-plus/prerm
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
     /etc/init.d/shadowsocksr disable
     /etc/init.d/shadowsocksr stop
fi
exit 0
endef

$(eval $(call BuildPackage,luci-app-ssr-plus))
