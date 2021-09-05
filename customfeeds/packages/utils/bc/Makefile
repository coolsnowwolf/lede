#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=bc
PKG_VERSION:=1.06.95
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).orig.tar.gz
PKG_SOURCE_URL:=ftp://ftp.debian.org/debian/pool/main/b/bc
PKG_HASH:=5e1471869dd27ba4120dd3942d2f4ec6646cf917fb056be9ae0d3a8259668d47

PKG_MAINTAINER:=Bruno Randolf <br1@einfach.org>
PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk

define Package/bc/Default
  SECTION:=utils
  CATEGORY:=Utilities
  URL:=http://packages.debian.org/bc
endef

define Package/bc
  $(call Package/bc/Default)
  TITLE:=Arbitrary precision calculator language
  DEPENDS:=+libreadline +libncurses
endef

define Package/bc/description
 bc is a language that supports arbitrary precision numbers with
 interactive execution of statements.
endef

define Package/dc
  $(call Package/bc/Default)
  TITLE:=Arbitrary precision reverse-polish calculator
  DEPENDS:=bc
endef

define Package/dc/description
 dc is a reverse-polish desk calculator which supports unlimited
 precision arithmetic.
endef

CONFIGURE_ARGS += --with-readline

define Package/bc/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bc/bc $(1)/usr/bin/
endef

define Package/dc/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dc/dc $(1)/usr/bin/
endef

$(eval $(call BuildPackage,bc))
$(eval $(call BuildPackage,dc))
