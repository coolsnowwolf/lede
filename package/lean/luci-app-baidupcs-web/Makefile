#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-baidupcs-web
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=[CTCGFW] Project OpenWRT

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=LuCI Support for BaiduPCS-Web.
	PKGARCH:=all
	DEPENDS:=+BaiduPCS-Web +wget +openssl-util
endef

define Package/$(PKG_NAME)/description
LuCI Support for BaiduPCS-Web.
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	( . /etc/uci-defaults/luci-baidupcs-web ) && rm -f /etc/uci-defaults/luci-baidupcs-web
fi
exit 0
endef

define Package/$(PKG_NAME)/conffiles
	/etc/config/baidupcs-web
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DATA) ./files/luci/controller/baidupcs-web.lua $(1)/usr/lib/lua/luci/controller/baidupcs-web.lua
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi
	$(INSTALL_DATA) ./files/luci/model/cbi/baidupcs-web.lua $(1)/usr/lib/lua/luci/model/cbi/baidupcs-web.lua
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/etc/config/baidupcs-web $(1)/etc/config/baidupcs-web
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/etc/init.d/baidupcs-web $(1)/etc/init.d/baidupcs-web
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/etc/uci-defaults/luci-baidupcs-web $(1)/etc/uci-defaults/luci-baidupcs-web
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
