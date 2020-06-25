#
# Copyright (C) 2019 OpenWrt.org
#
# Twinzo1 <1282055288@qq.com>
# KFERMercer <KFER.Mercer@gmail.com>
# 	
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/AdguardTeam/AdGuardHome.git

PKG_NAME:=AdGuardHome
PROJECT_NAME:=AdGuardHome
PKG_REV:=bab7b2dc65004d7c9661a590965f710584f063b9
PKG_VERSION:=0.102.0
PKG_RELEASE:=1
PKG_LICENSE:=GPL-3.0

PKG_SOURCE_VERSION:=$(PKG_REV)
PKG_SOURCE_SUBDIR:=$(PROJECT_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Network-wide ads & trackers blocking DNS server
	URL:=https://github.com/AdguardTeam/AdGuardHome
endef


define Package/$(PKG_NAME)/description
Network-wide ads & trackers blocking DNS server
endef

define Build/Configure
	mkdir -p $(PKG_INSTALL_DIR)
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) $(MAKE_ARGS) all
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PROJECT_NAME) $(1)/usr/bin/AdGuardHome/$(PKG_NAME)
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(1)/usr/bin/AdGuardHome/$(PKG_NAME)
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
