#
# Copyright (C) 2017 Roger Pueyo Centelles <roger.pueyo@guifi.net>
#
# This is free software, licensed under the GNU General Public License v3
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mikrotik_btest
PKG_VERSION:=0.5.1
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/kadosch/mikrotik_btest.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=02a0e7df411b5e7775fe2795709996a30afb7f47
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_MIRROR_HASH:=f72e1c8e2d98ca9e1c6a7fba35838a4a3c25b2dcd4bd8b10b67662003b3a14d1

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk

define Package/mikrotik-btest
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Bandwidth test client for Mikrotik BTest server
  DEPENDS:=+libpthread +librt
  URL:=https://github.com/kadosch/mikrotik_btest.git
  MAINTAINER:=Roger Pueyo Centelles <roger.pueyo@guifi.net>
endef

define Package/mikrotik-btest/description
	A bandwidth test client compatible with Mikrotik's RouterOS BTest server.
endef

define Package/mikrotik-btest/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mikrotik_btest $(1)/usr/bin/mikrotik_btest
endef

$(eval $(call BuildPackage,mikrotik-btest))
