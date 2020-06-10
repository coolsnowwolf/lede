#
# Copyright (C) 2012-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=ps3netsrv
PKG_VERSION:=20190630
PKG_GIT_COMMIT:=3107bba17870657c5bb3f7830657838d766a296c
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_GIT_COMMIT).tar.gz
PKG_SOURCE_URL:=https://github.com/jhonathanc/ps3netsrv/releases/download/$(PKG_VERSION)
PKG_HASH:=b1df52bab6496d3008019ff76660436c4b24d05c84fc71f293719dbd608f1511
PKG_INSTALL:=1

PKG_MAINTAINER:=JCorrea
PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=COPYING
PKG_BUILD_DEPENDS:=mbedtls

include $(INCLUDE_DIR)/package.mk

define Package/ps3netsrv
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=File Transfer
  TITLE:=PS3 Game Server
  URL:=https://github.com/jhonathanc/ps3netsrv-openwrt/
  DEPENDS:=+libpthread +libstdcpp
endef

define Package/ps3netsrv/description
ps3netsrv allows you to stream games and ISOs over the network to your CFW PlayStation(R) 3 system.
This may be useful if you have a modified PS3, and your router has storage options.
endef

define Build/Install
	# Do nothing
endef

define Package/ps3netsrv/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ps3netsrv $(1)/usr/bin

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/ps3netsrv.init $(1)/etc/init.d/ps3netsrv

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/ps3netsrv.config $(1)/etc/config/ps3netsrv
endef

$(eval $(call BuildPackage,ps3netsrv))
