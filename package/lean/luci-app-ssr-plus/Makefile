include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-ssr-plus
PKG_VERSION:=175
PKG_RELEASE:=3

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)/config
config PACKAGE_$(PKG_NAME)_INCLUDE_V2ray_plugin
	bool "Include Shadowsocks V2ray Plugin"
	default y if x86||x86_64||arm||aarch64

config PACKAGE_$(PKG_NAME)_INCLUDE_V2ray
	bool "Include V2ray"
	default y if x86||x86_64||arm||aarch64

config PACKAGE_$(PKG_NAME)_INCLUDE_Trojan
	bool "Include Trojan"
	default y if x86||x86_64||arm||aarch64
	
config PACKAGE_$(PKG_NAME)_INCLUDE_Redsocks2
	bool "Include Redsocks2"
	default y if x86||x86_64||arm||aarch64

config PACKAGE_$(PKG_NAME)_INCLUDE_Kcptun
	bool "Include Kcptun"
	default n

config PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Server
	bool "Include ShadowsocksR Server"
	default y if x86||x86_64||arm||aarch64
endef

define Package/$(PKG_NAME)
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=SS/SSR/V2Ray/Trojan LuCI interface
	PKGARCH:=all
	DEPENDS:=+shadowsocksr-libev-alt +ipset +ip-full +iptables-mod-tproxy +dnsmasq-full +coreutils +coreutils-base64 +pdnsd-alt +wget +lua +libuci-lua \
	+microsocks +dns2socks +shadowsocks-libev-ss-local +shadowsocksr-libev-ssr-local +shadowsocks-libev-ss-redir +simple-obfs +tcpping \
	+PACKAGE_$(PKG_NAME)_INCLUDE_V2ray_plugin:v2ray-plugin \
	+PACKAGE_$(PKG_NAME)_INCLUDE_V2ray:v2ray \
	+PACKAGE_$(PKG_NAME)_INCLUDE_Trojan:trojan \
	+PACKAGE_$(PKG_NAME)_INCLUDE_Trojan:ipt2socks \
	+PACKAGE_$(PKG_NAME)_INCLUDE_Redsocks2:redsocks2 \
	+PACKAGE_$(PKG_NAME)_INCLUDE_Kcptun:kcptun-client \
	+PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Server:shadowsocksr-libev-server
endef

define Build/Prepare
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/conffiles
/etc/ssr_ip
/etc/china_ssr.txt
/etc/config/shadowsocksr
/etc/config/white.list
/etc/config/black.list
/etc/config/netflix.list
/etc/dnsmasq.ssr/ad.conf
/etc/dnsmasq.ssr/gfw_list.conf
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) ./root/etc/china_ssr.txt $(1)/etc/china_ssr.txt

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./root/etc/config/shadowsocksr $(1)/etc/config/shadowsocksr
	$(INSTALL_DATA) ./root/etc/config/*.list $(1)/etc/config/

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

	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	po2lmo ./po/zh-cn/ssr-plus.po $(1)/usr/lib/lua/luci/i18n/ssr-plus.zh-cn.lmo
endef

define Package/$(PKG_NAME)/postrm
#!/bin/sh
rm -rf /etc/china_ssr.txt /etc/dnsmasq.ssr /etc/dnsmasq.oversea /etc/config/shadowsocksr /etc/config/black.list \
		/etc/config/gfw.list /etc/config/white.list /etc/config/netflix.list /etc/config/netflixip.list 2>/dev/null
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
