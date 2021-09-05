#
# Copyright (C) 2011 segal.ubi.pt
# Copyright (C) 2010-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ntripserver
PKG_VERSION:=1.51
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/nunojpg/ntripserver/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=3878c13d1b56e14085ce995565a2113aa592e3aa0b0eacfc65b6e3f5bdfc07e7

PKG_LICENSE:=GPL-2.0-or-later

PKG_MAINTAINER:=Nuno Goncalves <nunojpg@gmail.com>

include $(INCLUDE_DIR)/package.mk

define Package/ntripserver
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=NTRIP
  TITLE:=Ntrip Version 2.0 Command Line Server
  URL:=http://igs.bkg.bund.de/ntrip/download
endef

define Package/ntripserver/description
  Ntrip Version 2.0 Command Line Server, reading from SISNeT Server, TCP/UDP IP
  Port, Serial port, or NtripCaster to support an Ntrip Version 1.0 or 2.0 Caster
endef

MAKE_FLAGS += \
	OPTS="$(TARGET_CFLAGS)"

define Package/ntripserver/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ntripserver $(1)/usr/bin/
endef

$(eval $(call BuildPackage,ntripserver))
