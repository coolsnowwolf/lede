include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-shadowsocksr-alex
PKG_VERSION=1.7.62
PKG_RELEASE:=1
PKG_MAINTAINER:=Alex Zhuo <1886090@gmail.com>

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	PKGARCH:=all
	TITLE:=luci for shadowsocksR
	DEPENDS:=+shadowsocks-libev +dnsforwarder +ipset +ip +iptables-mod-tproxy +kmod-ipt-tproxy +iptables-mod-nat-extra +coreutils-nohup
endef

define Package/$(PKG_NAME)/description
    A luci app for shadowsocksR (Alex Zhuo)
endef

define Build/Prepare
	$(foreach po,$(wildcard ${CURDIR}/i18n/zh-cn/*.po), \
		po2lmo $(po) $(PKG_BUILD_DIR)/$(patsubst %.po,%.lmo,$(notdir $(po)));)
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/shadowsocksR.*.lmo $(1)/usr/lib/lua/luci/i18n/
	$(CP) ./files/* $(1)/

endef

$(eval $(call BuildPackage,$(PKG_NAME)))
