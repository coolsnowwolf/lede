include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-dogcom
PKG_VERSION:=1.0.2
PKG_RELEASE:=1

PKG_MAINTAINER:=fuyumi <280604399@qq.com>
PKG_LICENSE:=AGPLv3
PKG_LICENSE_FILES:=LICENSE

PKG_SOURCE_SUBDIR:=$(PKG_NAME)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/luci-app-dogcom
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=LuCI app for openwrt-dogcom
	DEPENDS:=+dogcom
	PKGARCH:=all
	MAINTAINER:=fuyumi
endef

define Package/luci-app-dogcom/description
	This package contains LuCI configuration pages for openwrt-dogcom.
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/luci-app-dogcom/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	( . /etc/uci-defaults/luci-app-dogcom ) && rm -f /etc/uci-defaults/luci-app-dogcom
	rm -f /tmp/luci-indexcache
fi
exit 0
endef

define Package/luci-app-dogcom/install
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/view/dogcom
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/init.d

	$(INSTALL_BIN) ./files/root/etc/uci-defaults/40_luci-dogcom $(1)/etc/uci-defaults/luci-app-dogcom
	$(INSTALL_DATA) ./files/luci/controller/dogcom.lua $(1)/usr/lib/lua/luci/controller/dogcom.lua
	$(INSTALL_DATA) ./files/luci/model/cbi/dogcom.lua $(1)/usr/lib/lua/luci/model/cbi/dogcom.lua
	$(INSTALL_DATA) ./files/luci/view/dogcom/auto_configure.htm $(1)/usr/lib/lua/luci/view/dogcom/auto_configure.htm
	$(INSTALL_DATA) ./files/luci/i18n/dogcom_zh-cn.lmo $(1)/usr/lib/lua/luci/i18n/dogcom.zh-cn.lmo
	$(INSTALL_CONF) ./files/root/etc/config/dogcom $(1)/etc/config/dogcom
	$(INSTALL_BIN) ./files/root/etc/init.d/dogcom $(1)/etc/init.d/dogcom
endef

$(eval $(call BuildPackage,luci-app-dogcom))
