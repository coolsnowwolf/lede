#
# Copyright (C) 2016-2023 GitHub 
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk

PKG_NAME:=default-settings
PKG_RELEASE:=$(COMMITCOUNT)
PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE

DEFAULT_SETTINGS_BUILD_DATE:=$(shell TZ=Asia/Shanghai date +%y.%-m.%-d)
PKG_CONFIG_DEPENDS:=DEFAULT_SETTINGS_BUILD_DATE

include $(INCLUDE_DIR)/package.mk

define Package/default-settings
  SECTION:=luci
  CATEGORY:=LuCI
  TITLE:=LuCI support for Default Settings
  PKGARCH:=all
  DEPENDS:= +luci-compat +@LUCI_LANG_zh-cn +@LUCI_LANG_zh_Hans +@LUCI_LANG_en
endef

define Package/default-settings/description
	Language Support Packages.
endef

define Build/Compile
endef

define Package/default-settings/install
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/zzz-default-settings $(1)/etc/uci-defaults/99-default-settings
	$(SED) 's/@BUILD_DATE@/$(DEFAULT_SETTINGS_BUILD_DATE)/g' $(1)/etc/uci-defaults/99-default-settings
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	po2lmo ./po/zh-cn/default.po $(1)/usr/lib/lua/luci/i18n/default.zh-cn.lmo
endef

$(eval $(call BuildPackage,default-settings))
