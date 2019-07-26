#
# Copyright (C) 2019 [CTCGFW] Project OpenWRT
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-filebrowser
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_LICENSE:=GPLv3
PKG_MAINTAINER:=[CTCGFW] Project OpenWRT

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=LuCI Support for FileBrowser
	PKGARCH:=all
	DEPENDS:=+FileBrowser +wget +openssl-util
endef

define Package/$(PKG_NAME)/description
	LuCI Support for FileBrowser.
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
	( . /etc/uci-defaults/luci-filebrowser ) && rm -f /etc/uci-defaults/luci-filebrowser
fi
exit 0
endef

define Package/$(PKG_NAME)/conffiles
	/etc/config/filebrowser
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DATA) ./files/luci/controller/filebrowser.lua $(1)/usr/lib/lua/luci/controller/filebrowser.lua
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi/filebrowser
	$(INSTALL_DATA) ./files/luci/model/cbi/filebrowser.lua $(1)/usr/lib/lua/luci/model/cbi/filebrowser.lua
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/etc/config/filebrowser $(1)/etc/config/filebrowser
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/etc/init.d/filebrowser $(1)/etc/init.d/filebrowser
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/etc/uci-defaults/luci-filebrowser $(1)/etc/uci-defaults/luci-filebrowser
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
