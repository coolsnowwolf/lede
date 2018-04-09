#
# Copyright (C) 2016-2017 Jian Chang <aa65535@live.com>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-udp2raw
PKG_VERSION:=1.0.0
PKG_RELEASE:=4

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Jian Chang <aa65535@live.com>

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=LuCI Support for udp2raw-tunnel
	PKGARCH:=all
#	DEPENDS:=+udp2raw-tunnel
endef

define Package/$(PKG_NAME)/description
	LuCI Support for udp2raw-tunnel.
endef

define Build/Prepare
	$(foreach po,$(wildcard ${CURDIR}/files/luci/i18n/*.po), \
		po2lmo $(po) $(PKG_BUILD_DIR)/$(patsubst %.po,%.lmo,$(notdir $(po)));)
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	( . /etc/uci-defaults/luci-udp2raw ) && rm -f /etc/uci-defaults/luci-udp2raw
fi
exit 0
endef

define Package/$(PKG_NAME)/conffiles
	/etc/config/udp2raw
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/udp2raw.*.lmo $(1)/usr/lib/lua/luci/i18n/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DATA) ./files/luci/controller/*.lua $(1)/usr/lib/lua/luci/controller/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi/udp2raw
	$(INSTALL_DATA) ./files/luci/model/cbi/udp2raw/*.lua $(1)/usr/lib/lua/luci/model/cbi/udp2raw/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/view/udp2raw
	$(INSTALL_DATA) ./files/luci/view/udp2raw/*.htm $(1)/usr/lib/lua/luci/view/udp2raw/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/root/etc/config/udp2raw $(1)/etc/config/udp2raw
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/root/etc/init.d/udp2raw $(1)/etc/init.d/udp2raw
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/root/etc/uci-defaults/luci-udp2raw $(1)/etc/uci-defaults/luci-udp2raw
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
