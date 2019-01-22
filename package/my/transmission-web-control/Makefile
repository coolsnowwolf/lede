#
# Copyright (C) 2018 OpenWrt-dist
# Copyright (C) 2018 dz
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=transmission-web-control
PKG_VERSION:=1.6.0-alpha
PKG_REV:=bade4d9
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/ronggang/transmission-web-control.git
PKG_SOURCE_VERSION:=$(PKG_REV)
PKG_SOURCE_SUBDIR:=$(PKG_NAME)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_REV).tar.gz

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=ronggang

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/transmission-web-control
    SUBMENU:=BitTorrent
    SECTION:=net
    CATEGORY:=Network
    TITLE:=transmission-web-control
    URL:=https://github.com/ronggang/transmission-web-control
    DEPENDS:=
endef

define Package/$(PKG_NAME)/description
transmission-web-control
endef



define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/share/transmission
	$(CP) -r $(PKG_BUILD_DIR)/src $(1)/usr/share/transmission/web
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
