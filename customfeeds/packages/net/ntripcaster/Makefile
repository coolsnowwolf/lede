#
# Copyright (C) 2011 segal.ubi.pt
# Copyright (C) 2010-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ntripcaster
PKG_VERSION:=0.1.5
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/nunojpg/ntripcaster/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=2184af9821cf73bac1df229f1e61ca1d3e288c9de6087bee1ae218b54c588452

PKG_LICENSE:=GPL-2.0-or-later

PKG_MAINTAINER:=Nuno Goncalves <nunojpg@gmail.com>

include $(INCLUDE_DIR)/package.mk

define Package/ntripcaster
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=NTRIP
  TITLE:=Standard Ntrip Broadcaster
  URL:=https://igs.bkg.bund.de/ntrip
  DEPENDS:=+libpthread
endef

define Package/ntripcaster/description
  BKG Standard Ntrip Broadcaster
endef

define Package/ntripcaster/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/ntripcaster $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/share/ntripcaster/conf
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/conf/ntripcaster.conf.dist $(1)/usr/share/ntripcaster/conf/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/conf/sourcetable.dat.dist $(1)/usr/share/ntripcaster/conf/
endef

$(eval $(call BuildPackage,ntripcaster))
