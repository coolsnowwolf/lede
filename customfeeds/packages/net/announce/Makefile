#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=announce
PKG_VERSION:=1.0.1
PKG_RELEASE:=1
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=src/LICENSE.txt
PKG_MAINTAINER:=Simon Peter <probono@puredarwin.org>

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/probonopd/announce.git
PKG_SOURCE_VERSION:=1368525c7305ca5bb4134242f332344f5f7e94e3
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_MIRROR_HASH:=247059922197db04e78c2845974d3dc2fc95a0624a0e0da32b5a13fd86a59d1b
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/announce
	SECTION:=net
	CATEGORY:=Network
	SUBMENU:=IP Addresses and Names
	TITLE:=Announce services on the network with Zeroconf/Bonjour 
	URL:=https://github.com/probonopd/announce
	DEPENDS:= +libpthread 
endef

define Package/announce/description
  Announce services on the network with Zeroconf/Bonjour.
  This announces services such as ssh, sftp, and http running on the local machine
  to the network.
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	$(CP) $(PKG_BUILD_DIR)/src/* $(PKG_BUILD_DIR)/
endef

define Package/announce/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/announce $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/announce.initscript $(1)/etc/init.d/announce
endef

$(eval $(call BuildPackage,announce))
