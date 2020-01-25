include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-ssr-plus
PKG_RELEASE:=136
PKG_VERSION:=1

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)/config
config PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks
	bool "Include Shadowsocks New Version"
	default y if x86_64

config PACKAGE_$(PKG_NAME)_INCLUDE_V2ray
	bool "Include V2ray"
	default y if x86_64

config PACKAGE_$(PKG_NAME)_INCLUDE_Trojan
	bool "Include Trojan"
	default y if x86_64

config PACKAGE_$(PKG_NAME)_INCLUDE_Kcptun
	bool "Include Kcptun"
	default n

config PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Server
	bool "Include ShadowsocksR Server"
	default y if x86_64

config PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Socks
	bool "Include ShadowsocksR Socks and Tunnel"
	default y if x86_64
endef

define Package/luci-app-ssr-plus
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=SS/SSR/V2Ray/Trojan LuCI interface
	PKGARCH:=all
	DEPENDS:=+shadowsocksr-libev-alt +ipset +ip-full +iptables-mod-tproxy +dnsmasq-full +coreutils +coreutils-base64 +pdnsd-alt +wget +lua \
	+PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks:shadowsocks-libev-ss-redir \
	+PACKAGE_$(PKG_NAME)_INCLUDE_V2ray:v2ray \
	+PACKAGE_$(PKG_NAME)_INCLUDE_Trojan:trojan \
	+PACKAGE_$(PKG_NAME)_INCLUDE_Trojan:ipt2socks \
	+PACKAGE_$(PKG_NAME)_INCLUDE_Kcptun:kcptun-client \
	+PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Server:shadowsocksr-libev-server \
	+PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Socks:shadowsocksr-libev-ssr-local
endef

define Build/Prepare
endef

define Build/Compile
endef

define Package/luci-app-ssr-plus/conffiles
/etc/ssr_ip
/etc/dnsmasq.ssr/gfw_list.conf
endef

define Package/luci-app-ssr-plus/install
	#lua
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi/shadowsocksr
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/view/shadowsocksr
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	#etc
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/dnsmasq.oversea
	$(INSTALL_DIR) $(1)/etc/dnsmasq.ssr
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	#usr
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/share/shadowsocksr
	#root/etc
	$(INSTALL_DATA) ./root/etc/china_ssr.txt $(1)/etc/china_ssr.txt
	$(INSTALL_CONF) ./root/etc/config/shadowsocksr $(1)/etc/config/shadowsocksr
	$(INSTALL_DATA) ./root/etc/config/*.list $(1)/etc/config/
	$(INSTALL_DATA) ./root/etc/dnsmasq.oversea/* $(1)/etc/dnsmasq.oversea/
	$(INSTALL_DATA) ./root/etc/dnsmasq.ssr/* $(1)/etc/dnsmasq.ssr/
	$(INSTALL_BIN) ./root/etc/init.d/* $(1)/etc/init.d/
	$(INSTALL_BIN) ./root/etc/uci-defaults/* $(1)/etc/uci-defaults/
	#root/usr
	$(INSTALL_BIN) ./root/usr/bin/* $(1)/usr/bin/
	$(INSTALL_BIN) ./root/usr/share/shadowsocksr/*.sh $(1)/usr/share/shadowsocksr/
	$(INSTALL_DATA) ./root/usr/share/shadowsocksr/*.lua $(1)/usr/share/shadowsocksr/
	#luasrc
	$(INSTALL_DATA) ./luasrc/controller/*.lua $(1)/usr/lib/lua/luci/controller/
	$(INSTALL_DATA) ./luasrc/model/cbi/shadowsocksr/*.lua $(1)/usr/lib/lua/luci/model/cbi/shadowsocksr/
	$(INSTALL_DATA) ./luasrc/view/shadowsocksr/* $(1)/usr/lib/lua/luci/view/shadowsocksr/
	po2lmo ./po/zh-cn/ssr-plus.po $(1)/usr/lib/lua/luci/i18n/ssr-plus.zh-cn.lmo
endef

define Package/luci-app-ssr-plus/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	( . /etc/uci-defaults/luci-ssr-plus ) && rm -f /etc/uci-defaults/luci-ssr-plus
	rm -f /tmp/luci-indexcache
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
