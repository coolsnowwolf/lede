#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=autoconf
PKG_VERSION:=2.70
PKG_RELEASE:=1

PKG_SOURCE_URL:=@GNU/autoconf
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_HASH:=fa9e227860d9d845c0a07f63b88c8d7a2ae1aa2345fb619384bb8accc19fecc6

PKG_MAINTAINER:=Heinrich Schuchardt <xypron.glpk@gmx.de>
PKG_LICENSE:=GPL-3.0-or-later

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/autoconf
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=autoconf
  URL:=https://www.gnu.org/software/autoconf/
  DEPENDS:=+m4 +perl +perlbase-data +perlbase-file +perlbase-getopt \
  +perlbase-dynaloader +perlbase-text
endef

define Package/autoconf/description
  Autoconf is an extensible package of M4 macros that produce shell scripts to
  automatically configure software source code packages.
endef

CONFIGURE_VARS += M4=m4 EMACS=no

define Package/autoconf/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/share/autoconf
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/autoconf/INSTALL \
	$(1)/usr/share/autoconf/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/autoconf/autom4te.cfg \
	$(1)/usr/share/autoconf/
	$(INSTALL_DIR) $(1)/usr/share/autoconf/Autom4te
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/autoconf/Autom4te/* \
	$(1)/usr/share/autoconf/Autom4te/
	$(INSTALL_DIR) $(1)/usr/share/autoconf/autoconf
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/autoconf/autoconf/* \
	$(1)/usr/share/autoconf/autoconf/
	$(INSTALL_DIR) $(1)/usr/share/autoconf/autoscan
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/autoconf/autoscan/* \
	$(1)/usr/share/autoconf/autoscan/
	$(INSTALL_DIR) $(1)/usr/share/autoconf/autotest
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/autoconf/autotest/* \
	$(1)/usr/share/autoconf/autotest/
	$(INSTALL_DIR) $(1)/usr/share/autoconf/m4sugar
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/autoconf/m4sugar/* \
	$(1)/usr/share/autoconf/m4sugar/
endef

$(eval $(call BuildPackage,autoconf))
