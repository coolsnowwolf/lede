# SPDX-Identifier-License: GPL-3.0-or-later
#
# Copyright (C) 2019 ElonH <elonhhuang@gmail.com>

include $(TOPDIR)/rules.mk

PKG_NAME:=rclone-webui-react
PKG_VERSION:=2.0.5
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).zip
PKG_SOURCE_URL:=https://github.com/rclone/rclone-webui-react/releases/download/v$(PKG_VERSION)/currentbuild.zip?
PKG_HASH:=afd6836ecc5c5a1161e25cb0633c1167eb5933bb5069545680d69fcba635f011

PKG_LICENSE:=MIT
PKG_LICENSE_FILE:=LICENSE
PKG_MAINTAINER:=ElonH <elonhhuang@gmail.com>

include $(INCLUDE_DIR)/package.mk

UNZIP_CMD:=unzip -q -d $(PKG_BUILD_DIR) $(DL_DIR)/$(PKG_SOURCE)

define Package/rclone-webui-react
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Cloud Manager
  TITLE:=A reactjs based web UI for rclone
  URL:=https://github.com/rclone/rclone-webui-react
  DEPENDS:=+rclone
  PKGARCH:=all
endef

define Package/rclone-webui-react/description
  A full fledged UI for the rclone cloud sync tool.
endef

define Build/Compile
endef

define Package/rclone-webui-react/install
	$(INSTALL_DIR) $(1)/www/rclone-webui-react
	$(CP) $(PKG_BUILD_DIR)/build/* $(1)/www/rclone-webui-react
endef

$(eval $(call BuildPackage,rclone-webui-react))
