# Copyright (C) 2018-2020 Lienol <lawlienol@gmail.com>
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-passwall
PKG_VERSION:=3.6
PKG_RELEASE:=41
PKG_DATE:=20200415

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)/config
menu "Configuration"

config PACKAGE_$(PKG_NAME)_INCLUDE_ipt2socks
	bool "Include ipt2socks"
	default y
	
config PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks
	bool "Include Shadowsocks Redir (ss-redir)"
	default n
	
config PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR
	bool "Include ShadowsocksR Redir (ssr-redir)"
	default y

config PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks_socks
	bool "Include Shadowsocks Socks (ss-local)"
	default n
	
config PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_socks
	bool "Include ShadowsocksR Socks (ssr-local)"
	default y
	
config PACKAGE_$(PKG_NAME)_INCLUDE_V2ray
	bool "Include V2ray"
	default y

config PACKAGE_$(PKG_NAME)_INCLUDE_Trojan
	bool "Include Trojan"
	default n
	
config PACKAGE_$(PKG_NAME)_INCLUDE_Brook
	bool "Include Brook"
	default n
	
config PACKAGE_$(PKG_NAME)_INCLUDE_kcptun
	bool "Include kcptun"
	default n

config PACKAGE_$(PKG_NAME)_INCLUDE_haproxy
	bool "Include haproxy"
	default y
	
config PACKAGE_$(PKG_NAME)_INCLUDE_ChinaDNS_NG
	bool "Include ChinaDNS-NG"
	default y
	
config PACKAGE_$(PKG_NAME)_INCLUDE_pdnsd
	bool "Include pdnsd"
	default y
	
config PACKAGE_$(PKG_NAME)_INCLUDE_dns2socks
	bool "Include dns2socks"
	default y
	
config PACKAGE_$(PKG_NAME)_INCLUDE_v2ray-plugin
	bool "Include v2ray-plugin (Shadowsocks plugin)"
	default n

config PACKAGE_$(PKG_NAME)_INCLUDE_simple-obfs
	bool "Include simple-obfs (Shadowsocks plugin)"
	default n

endmenu
endef

define Package/$(PKG_NAME)
  CATEGORY:=LuCI
  SUBMENU:=3. Applications
  TITLE:=LuCI support for PassWall
  PKGARCH:=all
  DEPENDS:=+libmbedtls +iptables-mod-tproxy +ip +ipset +coreutils +coreutils-base64 +coreutils-nohup +luci-lib-jsonc \
  +wget +resolveip +unzip +dnsmasq-full +tcping +libuci-lua \
  +PACKAGE_$(PKG_NAME)_INCLUDE_ipt2socks:ipt2socks \
  +PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks:shadowsocks-libev-ss-redir \
  +PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR:shadowsocksr-libev-alt \
  +PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks_socks:shadowsocks-libev-ss-local \
  +PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_socks:shadowsocksr-libev-ssr-local \
  +PACKAGE_$(PKG_NAME)_INCLUDE_V2ray:v2ray \
  +PACKAGE_$(PKG_NAME)_INCLUDE_Trojan:trojan \
  +PACKAGE_$(PKG_NAME)_INCLUDE_Trojan:ipt2socks \
  +PACKAGE_$(PKG_NAME)_INCLUDE_Brook:brook \
  +PACKAGE_$(PKG_NAME)_INCLUDE_kcptun:kcptun-client \
  +PACKAGE_$(PKG_NAME)_INCLUDE_haproxy:haproxy \
  +PACKAGE_$(PKG_NAME)_INCLUDE_ChinaDNS_NG:chinadns-ng \
  +PACKAGE_$(PKG_NAME)_INCLUDE_pdnsd:pdnsd-alt \
  +PACKAGE_$(PKG_NAME)_INCLUDE_dns2socks:dns2socks \
  +PACKAGE_$(PKG_NAME)_INCLUDE_v2ray-plugin:v2ray-plugin \
  +PACKAGE_$(PKG_NAME)_INCLUDE_simple-obfs:simple-obfs
endef

define Build/Prepare
endef
 
define Build/Configure
endef
 
define Build/Compile
endef

define Package/$(PKG_NAME)/conffiles
/etc/config/passwall
/usr/share/passwall/rules/blacklist_host
/usr/share/passwall/rules/blacklist_ip
/usr/share/passwall/rules/whitelist_host
/usr/share/passwall/rules/whitelist_ip
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./root/etc/config/passwall $(1)/etc/config/passwall
	$(INSTALL_CONF) ./root/etc/config/passwall_show $(1)/etc/config/passwall_show
	
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_CONF) ./root/etc/uci-defaults/* $(1)/etc/uci-defaults
	
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./root/etc/init.d/passwall $(1)/etc/init.d/passwall
	
	$(INSTALL_DIR) $(1)/usr/share/passwall
	cp -pR ./root/usr/share/passwall/* $(1)/usr/share/passwall
	
	$(INSTALL_DIR) $(1)/usr/share/rpcd/acl.d
	cp -pR ./root/usr/share/rpcd/acl.d/* $(1)/usr/share/rpcd/acl.d
	
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci
	cp -pR ./luasrc/* $(1)/usr/lib/lua/luci/
	
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	po2lmo ./po/zh-cn/passwall.po $(1)/usr/lib/lua/luci/i18n/passwall.zh-cn.lmo
endef

define Package/$(PKG_NAME)/postinst
#!/bin/sh
chmod a+x $${IPKG_INSTROOT}/usr/share/passwall/* >/dev/null 2>&1
exit 0
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
