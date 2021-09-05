#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mutt
PKG_VERSION:=1.14.7
PKG_RELEASE:=2

PKG_SOURCE_URL:=https://bitbucket.org/mutt/mutt/downloads/ \
		http://ftp.mutt.org/pub/mutt/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=e4f507b133253cb5eef27996b8668956cdf9caac622cf8adad13f0f9a4eda864

PKG_MAINTAINER:=Phil Eichinger <phil@zankapfel.net>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=GPL
PKG_CPE_ID:=cpe:/a:mutt:mutt

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/mutt
  SECTION:=mail
  CATEGORY:=Mail
  DEPENDS:=+libopenssl +libncursesw +terminfo +zlib
  TITLE:=Console mail client
  URL:=http://www.mutt.org/
endef

define Package/mutt/description
	Mutt is a small but very powerful text-based mail client for Unix
	operating systems.
endef

CONFIGURE_ARGS += \
	--includedir=$(PKG_BUILD_DIR)/. \
	--oldincludedir=$(PKG_BUILD_DIR)/. \
	--enable-pop \
	--enable-imap \
	--with-mailpath=/var/mail \
	--with-ssl \
	--without-idn \
	--disable-doc

define Package/mutt/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/mutt $(1)/usr/bin/
endef

$(eval $(call BuildPackage,mutt))
