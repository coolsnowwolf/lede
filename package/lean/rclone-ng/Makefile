####
 #  File: /rclone-ng/Makefile
 #  Project: Rclone-OpenWrt
 #  File Created: Friday, 5th June 2020 12:37:26 am
 #  Author: ElonH[EH](elonhhuang@gmail.com)
 #  License: GNU General Public License v3.0 or later(http://www.gnu.org/licenses/gpl-3.0-standalone.html)
 #  Copyright (C) 2020 [ElonH]
####

include $(TOPDIR)/rules.mk

PKG_NAME:=rclone-ng
PKG_VERSION:=0.5.0
PKG_RELEASE:=1


PKG_LICENSE:=GPLv3
PKG_MAINTAINER:=ElonH <elonhhuang@gmail.com>

include $(INCLUDE_DIR)/package.mk

PKG_SOURCE:=RcloneNg-v$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/ElonH/RcloneNg/releases/download/v$(PKG_VERSION)/
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_HASH:=0b4916ddd0bacb5b358dc8d36b64c30f4182c0ace3eb06cda94b6578c419dcd9

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	SUBMENU:=File Transfer
	SUBMENU:=Cloud Manager
	TITLE:=An angular web application for rclone
	URL:=https://github.com/ElonH/RcloneNg
	PKGARCH:=all
endef

define Package/$(PKG_NAME)/description
	An angular web application for rclone
endef

define Build/Prepare
	mkdir -vp $(PKG_BUILD_DIR)
	tar -xzf $(DL_DIR)/$(PKG_SOURCE) -C $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Build/Compile
	echo "$(PKG_NAME) Compile Skiped!"
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/www
	$(CP) $(PKG_BUILD_DIR)/RcloneNg $(1)/www
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
