#
# Copyright (C) 2006-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=eqos
PKG_RELEASE:=1
LUCI_DIR:=/usr/lib/lua/luci

PKG_MAINTAINER:=Jianhui Zhao <jianhuizhao329@gmail.com>

include $(INCLUDE_DIR)/package.mk

define Package/eqos
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+tc +kmod-sched-core +kmod-ifb
  TITLE:=Easy QoS(Support speed limit based on IP address)
  PKGARCH:=all
endef


define Package/luci-app-eqos
  SECTION:=luci
  CATEGORY:=LuCI
  TITLE:=EQOS - LuCI interface
  PKGARCH:=all
  DEPENDS:=+luci-base +eqos
  SUBMENU:=3. Applications
endef

define Package/luci-app-eqos/description
	Luci interface for the eqos.
endef

define Package/luci-i18n-eqos-zh-cn
	SECTION:=luci
	CATEGORY:=LuCI
	TITLE:=luci-app-eqos - zh-cn translation
	HIDDEN:=1
	DEPENDS:=luci-app-eqos
	DEFAULT:=LUCI_LANG_zh-cn
    PKGARCH:=all
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/eqos/install
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/etc/config $(1)/etc/init.d $(1)/etc/hotplug.d/iface
	$(INSTALL_BIN) ./files/eqos.sh $(1)/usr/sbin/eqos
	$(INSTALL_BIN) ./files/eqos.init $(1)/etc/init.d/eqos
	$(INSTALL_BIN) ./files/eqos.hotplug $(1)/etc/hotplug.d/iface/10-eqos
	$(INSTALL_CONF) ./files/eqos.config $(1)/etc/config/eqos
endef

define Package/luci-app-eqos/install
	$(INSTALL_DIR) $(1)$(LUCI_DIR)/controller $(1)$(LUCI_DIR)/model/cbi $(1)$(LUCI_DIR)/i18n  $(1)/etc/uci-defaults
	$(INSTALL_DATA) ./files/eqos-controller.lua $(1)$(LUCI_DIR)/controller/eqos.lua
	$(INSTALL_DATA) ./files/eqos-cbi.lua $(1)$(LUCI_DIR)/model/cbi/eqos.lua
	$(INSTALL_BIN) ./files/uci-defaults-eqos $(1)/etc/uci-defaults/luci-eqos
endef

define Package/luci-app-eqos/postinst
#!/bin/sh
which uci > /dev/null || exit 0
uci -q get ucitrack.@eqos[0] > /dev/null || {
  uci add ucitrack eqos > /dev/null
  uci set ucitrack.@eqos[0].init=eqos
  uci commit
}
endef

define Package/luci-app-eqos/postrm
#!/bin/sh
which uci > /dev/null || exit 0
uci -q get ucitrack.@eqos[0] > /dev/null && {
  uci delete ucitrack.@eqos[0]
  uci commit
}
endef

define Package/luci-i18n-eqos-zh-cn/install
	$(INSTALL_DIR) $(1)$(LUCI_DIR)/i18n
	po2lmo ./files/po/zh-cn/eqos.po $(1)$(LUCI_DIR)/i18n/eqos.zh-cn.lmo
endef

$(eval $(call BuildPackage,eqos))
$(eval $(call BuildPackage,luci-app-eqos))
$(eval $(call BuildPackage,luci-i18n-eqos-zh-cn))