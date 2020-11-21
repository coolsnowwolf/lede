####
 #  File: /Makefile
 #  Project: rclone-webui-react
 #  File Created: Thursday, 10th October 2019 5:57:39 pm
 #  Author: ElonH[EH](elonhhuang@gmail.com)
 #  License: GNU General Public License v3.0 or later(http://www.gnu.org/licenses/gpl-3.0-standalone.html)
 #	Copyright (C) 2019 [ElonH]
####

include $(TOPDIR)/rules.mk

PKG_NAME:=rclone-webui-react
PKG_VERSION:=0.1.0
PKG_RELEASE:=1


PKG_LICENSE:=GPLv3
PKG_MAINTAINER:=ElonH <elonhhuang@gmail.com>

include $(INCLUDE_DIR)/package.mk

PKG_SOURCE:=currentbuild.zip
PKG_SOURCE_URL:=https://github.com/rclone/rclone-webui-react/releases/download/v$(PKG_VERSION)/
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_HASH:=06b9401779f82ef62fd22a9688549664228797d094b645a68a098c2310461fc5

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	SUBMENU:=File Transfer
	SUBMENU:=Cloud Manager
	TITLE:=A reactjs based web UI for rclone
	URL:=https://github.com/rclone/rclone-webui-react
	PKGARCH:=all
endef

define Package/$(PKG_NAME)/description
	A full fledged UI for the rclone cloud sync tool.
endef

define Build/Prepare
	mkdir -vp $(PKG_BUILD_DIR)
	unzip -od $(PKG_BUILD_DIR) $(DL_DIR)/$(PKG_SOURCE)
endef

define Build/Configure
endef

define Build/Compile
	echo "$(PKG_NAME) Compile Skiped!"
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/www/rclone-webui-react
	$(CP) $(PKG_BUILD_DIR)/build/* $(1)/www/rclone-webui-react
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
