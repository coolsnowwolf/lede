include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-vssr
PKG_VERSION:=1.22-2
PKG_RELEASE:=20210904

PKG_CONFIG_DEPENDS:= \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_Xray \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_Trojan \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_Kcptun \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_Xray_plugin \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Libev_Server
	
LUCI_TITLE:=A New SS/SSR/Xray/Trojan LuCI interface
LUCI_PKGARCH:=all
LUCI_DEPENDS:=+ipset +ip-full +iptables-mod-tproxy +dnsmasq-full +coreutils +coreutils-base64 +bash +pdnsd-alt +wget-ssl +lua +luasocket +lua-maxminddb \
	+shadowsocks-libev-ss-local +shadowsocks-libev-ss-redir +shadowsocksr-libev-ssr-local +shadowsocksr-libev-ssr-redir +shadowsocksr-libev-ssr-check +simple-obfs \
	+PACKAGE_$(PKG_NAME)_INCLUDE_Xray:xray-core \
	+PACKAGE_$(PKG_NAME)_INCLUDE_Trojan:trojan \
	+PACKAGE_$(PKG_NAME)_INCLUDE_Trojan:ipt2socks \
	+PACKAGE_$(PKG_NAME)_INCLUDE_Kcptun:kcptun-client \
	+PACKAGE_$(PKG_NAME)_INCLUDE_Xray_plugin:xray-plugin \
	+PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Libev_Server:shadowsocksr-libev-ssr-server

define Package/$(PKG_NAME)/config
config PACKAGE_$(PKG_NAME)_INCLUDE_Xray
	bool "Include Xray"
	default y if i386||x86_64||arm||aarch64

config PACKAGE_$(PKG_NAME)_INCLUDE_Trojan
	bool "Include Trojan"
	default y if i386||x86_64||arm||aarch64
	
config PACKAGE_$(PKG_NAME)_INCLUDE_Kcptun
	bool "Include Kcptun"
	default n

config PACKAGE_$(PKG_NAME)_INCLUDE_Xray_plugin
	bool "Include Shadowsocks Xray Plugin"
	default y if i386||x86_64||arm||aarch64
	
config PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Libev_Server
	bool "Include ShadowsocksR Libev Server"
	default y if i386||x86_64||arm||aarch64
endef

define Package/$(PKG_NAME)/conffiles
/etc/vssr/
/etc/config/vssr
endef

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
