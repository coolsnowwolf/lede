# SPDX-License-Identifier: GPL-3.0-or-later
#
# Copyright (C) 2020 ElonH <elonhhuang@gmail.com>

include $(TOPDIR)/rules.mk

PKG_NAME:=rclone-ng
PKG_VERSION:=0.5.0
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=RcloneNg-v$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/ElonH/RcloneNg/releases/download/v$(PKG_VERSION)/
PKG_HASH:=0b4916ddd0bacb5b358dc8d36b64c30f4182c0ace3eb06cda94b6578c419dcd9

PKG_LICENSE:=MIT
PKG_LICENSE_FILE:=LICENSE
PKG_MAINTAINER:=ElonH <elonhhuang@gmail.com>

include $(INCLUDE_DIR)/package.mk

TAR_CMD:=$(HOST_TAR) -C $(PKG_BUILD_DIR) $(TAR_OPTIONS)

define Package/rclone-ng
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Cloud Manager
  TITLE:=An angular web application for rclone
  URL:=https://github.com/ElonH/RcloneNg
  DEPENDS:=+rclone
  PKGARCH:=all
endef

define Build/Compile
endef

define Package/rclone-ng/install
	$(INSTALL_DIR) $(1)/www
	$(CP) $(PKG_BUILD_DIR)/RcloneNg $(1)/www
endef

$(eval $(call BuildPackage,rclone-ng))
