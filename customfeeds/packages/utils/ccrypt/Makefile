#
# Copyright (C) 2009-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ccrypt
PKG_VERSION:=1.11
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/ccrypt
PKG_HASH:=b19c47500a96ee5fbd820f704c912f6efcc42b638c0a6aa7a4e3dc0a6b51a44f
PKG_MAINTAINER:=Hannu Nyman <hannu.nyman@iki.fi>
PKG_LICENSE:=GPL-2.0-or-later

PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk

define Package/ccrypt
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Encryption
  TITLE:=utility for encrypting/decrypting files and streams
  URL:=http://ccrypt.sourceforge.net/
endef

CONFIGURE_ARGS += --disable-emacs

define Package/ccrypt/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/ccrypt $(1)/usr/bin/
	ln -s ccrypt $(1)/usr/bin/ccencrypt
	ln -s ccrypt $(1)/usr/bin/ccdecrypt
	ln -s ccrypt $(1)/usr/bin/ccat
endef

$(eval $(call BuildPackage,ccrypt))
