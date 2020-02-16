#
# Copyright (C) 2015-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
#
# licheng
# www.maxlicheng.com
# 2019-07-28
#

include $(TOPDIR)/rules.mk

PKG_NAME:=UnblockNeteaseMusic
PKG_VERSION:=0.23.1
PKG_RELEASE:=1

PKG_LICENSE:=MIT

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/nondanee/UnblockNeteaseMusic.git
PKG_SOURCE_VERSION:=1251b9d96826ce5bb63131d0f18696253ec02127

PKG_SOURCE_SUBDIR:=$(PKG_NAME)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Revive unavailable songs for Netease Cloud Music
	DEPENDS:=+node 
	URL:=https://github.com/nondanee/UnblockNeteaseMusic/releases
	SUBMENU:=NeteaseMusic
	PKGARCH:=all
endef

define Package/$(PKG_NAME)/description
Revive unavailable songs for Netease Cloud Music
endef

define Build/Prepare
	tar -xzvf $(DL_DIR)/$(PKG_SOURCE) -C $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)/$(PKG_NAME)
	echo -e $(PKG_VERSION) > $(PKG_BUILD_DIR)/$(PKG_NAME)/core_ver
	echo -e $(PKG_SOURCE_VERSION) > $(PKG_BUILD_DIR)/$(PKG_NAME)/local_ver
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR)  $(1)/usr/share/$(PKG_NAME)
	cp -pR $(PKG_BUILD_DIR)/$(PKG_NAME)/* $(1)/usr/share/$(PKG_NAME)
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
