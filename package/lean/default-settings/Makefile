#
# Copyright (C) 2016-2017 GitHub 
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk

PKG_NAME:=default-settings
PKG_VERSION:=1.0
PKG_RELEASE:=55
PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/default-settings
  SECTION:=luci
  CATEGORY:=LuCI
  TITLE:=LuCI support for Default Settings
  PKGARCH:=all
  DEPENDS:=+@LUCI_LANG_zh-cn
endef

define Package/default-settings/description
	Language Support Packages.
endef

define Build/Prepare
	$(foreach po,$(wildcard ${CURDIR}/i18n/*.po), \
		po2lmo $(po) $(PKG_BUILD_DIR)/$(patsubst %.po,%.lmo,$(notdir $(po)));)
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/default-settings/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/*.lmo $(1)/usr/lib/lua/luci/i18n/
	$(INSTALL_BIN) ./files/zzz-default-settings $(1)/etc/uci-defaults/99-default-settings
endef

$(eval $(call BuildPackage,default-settings))
