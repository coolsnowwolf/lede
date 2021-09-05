#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=perl-encode-locale
PKG_VERSION:=1.05
PKG_RELEASE:=2

PKG_SOURCE_URL:=http://www.cpan.org/authors/id/G/GA/GAAS
PKG_SOURCE:=Encode-Locale-$(PKG_VERSION).tar.gz
PKG_HASH:=176fa02771f542a4efb1dbc2a4c928e8f4391bf4078473bd6040d8f11adb0ec1

PKG_LICENSE:=GPL-1.0-or-later Artistic-1.0-Perl
PKG_MAINTAINER:=Marcel Denia <naoir@gmx.net>

PKG_BUILD_DIR:=$(BUILD_DIR)/perl/Encode-Locale-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include ../perl/perlmod.mk

define Package/perl-encode-locale
  SUBMENU:=Perl
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Determine the locale encoding
  URL:=http://search.cpan.org/dist/Encode-Locale/
  DEPENDS:=perl +perlbase-base +perlbase-encode +perlbase-essential
endef

define Build/Configure
        $(call perlmod/Configure,,)
endef

define Build/Compile
        $(call perlmod/Compile,,)
endef

define Package/perl-encode-locale/install
        $(call perlmod/Install,$(1),Encode auto/Encode)
endef


$(eval $(call BuildPackage,perl-encode-locale))
