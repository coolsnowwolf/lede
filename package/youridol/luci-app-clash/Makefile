include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-clash
PKG_VERSION:=0.1.0
PKG_RELEASE:=1

PKG_MAINTAINER:=Anton Chen <contact@antonchen.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_SOURCE_SUBDIR:=$(PKG_NAME)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/luci-app-clash
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=2. Services
	TITLE:=LuCI app for clash
	DEPENDS:=+clash +luci-lib-json +libuci-lua +coreutils-nohup +shadow-su +iptables-mod-extra +curl
	PKGARCH:=all
	MAINTAINER:=Anton
endef

define Package/luci-app-clash/description
	This package contains LuCI configuration pages for clash.
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/luci-app-clash/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi/clash
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/view/clash
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/clash
	$(INSTALL_DIR) $(1)/usr/share/clash

	$(INSTALL_BIN) files/etc/init.d/clash $(1)/etc/init.d/clash
	$(INSTALL_CONF) files/etc/config/clash $(1)/etc/config/clash
	$(INSTALL_CONF) files/etc/clash/custom-proxy $(1)/etc/clash/custom-proxy
	$(INSTALL_CONF) files/etc/clash/custom-group $(1)/etc/clash/custom-group
	$(INSTALL_CONF) files/etc/clash/custom-select $(1)/etc/clash/custom-select
	$(INSTALL_CONF) files/etc/clash/custom-rule $(1)/etc/clash/custom-rule

	$(INSTALL_BIN) files/usr/share/clash/gen-clash-config.sh $(1)/usr/share/clash/gen-clash-config.sh
	$(INSTALL_BIN) files/usr/share/clash/subscription-to-clash.lua $(1)/usr/share/clash/subscription-to-clash.lua
	$(INSTALL_BIN) files/usr/share/clash/update-mmdb.sh $(1)/usr/share/clash/update-mmdb.sh

	$(INSTALL_DATA) luasrc/controller/clash.lua $(1)/usr/lib/lua/luci/controller/clash.lua
	$(INSTALL_DATA) luasrc/model/cbi/clash/general.lua $(1)/usr/lib/lua/luci/model/cbi/clash/general.lua
	$(INSTALL_DATA) luasrc/model/cbi/clash/subscription.lua $(1)/usr/lib/lua/luci/model/cbi/clash/subscription.lua
	$(INSTALL_DATA) luasrc/model/cbi/clash/custom-config.lua $(1)/usr/lib/lua/luci/model/cbi/clash/custom-config.lua
	$(INSTALL_DATA) luasrc/view/clash/status.htm $(1)/usr/lib/lua/luci/view/clash/status.htm
	$(INSTALL_DATA) luasrc/view/clash/view-config.htm $(1)/usr/lib/lua/luci/view/clash/view-config.htm
	$(INSTALL_DATA) luasrc/view/clash/log.htm $(1)/usr/lib/lua/luci/view/clash/log.htm

	$(INSTALL_DATA) po/zh-cn/clash.lmo $(1)/usr/lib/lua/luci/i18n/clash.zh-cn.lmo
endef

$(eval $(call BuildPackage,luci-app-clash))
