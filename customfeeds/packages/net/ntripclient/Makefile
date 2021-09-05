#
# Copyright (C) 2011 segal.ubi.pt
# Copyright (C) 2010-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ntripclient
PKG_VERSION:=1.51
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/nunojpg/ntripclient/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=d668e87b2260ef262e2066ac0ed217ec77478170deaf76ff48f2116a23c48ae5

PKG_LICENSE:=GPL-2.0-or-later

PKG_MAINTAINER:=Nuno Goncalves <nunojpg@gmail.com>

include $(INCLUDE_DIR)/package.mk

define Package/ntripclient
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=NTRIP
  TITLE:=Ntrip Version 2.0 Command Line Client
  URL:=http://igs.bkg.bund.de/ntrip/download
endef

define Package/ntripclient/description
  Ntrip Version 2.0 Command Line Client, reading from Ntrip Version 1.0 or 2.0 Caster
endef

MAKE_FLAGS += \
	OPTS="$(TARGET_CFLAGS)"

define Package/ntripclient/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ntripclient $(1)/usr/bin/
endef

$(eval $(call BuildPackage,ntripclient))
