#
# Copyright (C) 2006-2011 Xmlad.com
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=minieap
PKG_VERSION:=0.9
PKG_RELEASE:=2
PKG_REV=71742ef

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-r$(PKG_REV).tar.bz2
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=$(PKG_REV)
PKG_SOURCE_URL:=https://github.com/updateing/minieap.git

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/minieap
	SECTION:=net
	CATEGORY:=Network
	DEPENDS:=+libpcap
	TITLE:=Extendable 802.1x client
	URL:=https://github.com/updateing/minieap
	SUBMENU:=CERNET
endef

define Package/minieap/description
Extendable 802.1x client
endef

define Build/Prepare
	$(call Build/Prepare/Default)
endef

define Build/Install
	true
endef

define Package/minieap/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/minieap $(1)/usr/sbin/
	chmod 755 $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,minieap))
