#
# Copyright (C) 2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk

PKG_NAME:=ncp
PKG_VERSION:=1.2.4
PKG_RELEASE:=1

PKG_LICENSE:=FREE
PKG_LICENSE_FILES:=

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://dl.fefe.de/
PKG_HASH:=6cfa72edd5f7717bf7a4a93ccc74c4abd89892360e2e0bb095a73c24b9359b88

PKG_BUILD_DEPENDS:=libowfat

PKG_MAINTAINER:=Andreas Shimokawa <shimokawa@fsfe.org>

include $(INCLUDE_DIR)/package.mk

define Package/ncp
  SECTION:=net
  CATEGORY:=Network
  TITLE:=copy files over the network
  URL:=https://www.fefe.de/ncp/
endef

define Package/ncp/description
	copy files over the network
endef

define Build/Compile
	cd $(PKG_BUILD_DIR); \
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_LDFLAGS) -I$(STAGING_DIR)/usr/include/libowfat ncp.c -lowfat -o ncp
endef

define Package/ncp/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ncp $(1)/usr/bin/
	ln -sf ncp $(1)/usr/bin/npush
	ln -sf ncp $(1)/usr/bin/npoll
endef

$(eval $(call BuildPackage,ncp))
