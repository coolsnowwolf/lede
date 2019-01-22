#
# Copyright (C) 2016-2017 Jian Chang <aa65535@live.com>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-macvlan
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Chen Minqiang <ptpt52@gmail.com>

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/luci-app-macvlan
	CATEGORY:=Ptpt52
	SUBMENU:=Macvlan
	TITLE:=LuCI Support for macvlan
	PKGARCH:=all
	DEPENDS:=+kmod-macvlan
endef

define Package/luci-app-macvlan/description
	LuCI Support for macvlan.
endef

define Build/Prepare
	$(foreach po,$(wildcard ${CURDIR}/files/luci/i18n/*.po), \
		po2lmo $(po) $(PKG_BUILD_DIR)/$(patsubst %.po,%.lmo,$(notdir $(po)));)
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/luci-app-macvlan/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	rm -rf /tmp/luci-indexcache /tmp/luci-modulecache
fi

if [ -z "$$IPKG_INSTROOT" ]; then
  ( . /etc/uci-defaults/40_luci-app-macvlan )
  rm -f /etc/uci-defaults/40_luci-app-macvlan
fi

exit 0
endef

define Package/luci-app-macvlan/conffiles
/etc/config/macvlan
endef

define Package/luci-app-macvlan/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/macvlan.*.lmo $(1)/usr/lib/lua/luci/i18n/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DATA) ./files/luci/controller/*.lua $(1)/usr/lib/lua/luci/controller/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi/macvlan
	$(INSTALL_DATA) ./files/luci/model/cbi/macvlan/*.lua $(1)/usr/lib/lua/luci/model/cbi/macvlan/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/root/etc/config/macvlan $(1)/etc/config/macvlan
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/root/etc/init.d/macvlan $(1)/etc/init.d/macvlan
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_DATA) ./files/root/etc/uci-defaults/40_luci-app-macvlan $(1)/etc/uci-defaults/40_luci-app-macvlan
endef

$(eval $(call BuildPackage,luci-app-macvlan))
