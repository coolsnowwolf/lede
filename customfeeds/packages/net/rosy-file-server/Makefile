#
# Copyright (C) 2019 rosysong@rosinson.com
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=rosy-file-server
PKG_VERSION:=1.0.0
PKG_RELEASE:=1
PKG_LICENSE:=GPL-2.0

PKG_MAINTAINER:=Rosy Song <rosysong@rosinson.com>

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/rosy-file-server
  SUBMENU:=File Transfer
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Rosy File Server over HTTP
  PKGARCH:=all
endef

define Package/rosy-file-server/description
 This package is a configuration management for luci-app-rosy-file-server.
endef

define Package/rosy-file-server/conffiles
/etc/config/rosy-file-server
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/rosy-file-server/install
	$(INSTALL_DIR) $(1)/etc/init.d $(1)/etc/config
	$(INSTALL_BIN) ./files/rosyfs.init $(1)/etc/init.d/rosyfs
	$(INSTALL_CONF) ./files/rosyfs.config $(1)/etc/config/rosyfs
endef

$(eval $(call BuildPackage,rosy-file-server))
