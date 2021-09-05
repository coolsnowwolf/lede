#
# Copyright (C) 2010 segal.ubi.pt
# Copyright (C) 2014 nunojpg@gmail.com
# Copyright (C) 2015 dev@localnet.hu
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=open2300
PKG_SOURCE_DATE:=2014-03-04
PKG_SOURCE_VERSION:=1af8ae609da66f8e1b745533c19095c9758bfb0b
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/wezm/open2300
PKG_MIRROR_HASH:=3287ca1c70e736dd1c6c3abb01e0d986f91573c7c54b9adabdc437309dc4fd56

PKG_MAINTAINER:=Gabor SZOLLOSI <dev@localnet.hu>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk

define Package/open2300
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=WS23XX weather station utilities
  URL:=http://www.lavrsen.dk/foswiki/bin/view/Open2300
endef

define Package/open2300/description
  This is an utility suite used to communicate with and collect data from a WS23XX wheather station.
endef

define Package/open2300/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin2300		$(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cw2300		$(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dump2300	$(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/fetch2300	$(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/histlog2300	$(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/history2300	$(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/interval2300	$(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/light2300	$(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/log2300		$(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/minmax2300	$(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/open2300	$(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wu2300		$(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/xml2300		$(1)/usr/bin/

	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) ./files/open2300.conf $(1)/etc/

	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/lib2300.so.1.11 $(1)/usr/lib/
	$(LN) lib2300.so.1.11 $(1)/usr/lib/lib2300.so
endef

$(eval $(call BuildPackage,open2300))
