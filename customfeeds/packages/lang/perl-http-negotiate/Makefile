#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=perl-http-negotiate
PKG_VERSION:=6.01
PKG_RELEASE:=2

PKG_SOURCE_URL:=http://www.cpan.org/authors/id/G/GA/GAAS
PKG_SOURCE:=HTTP-Negotiate-$(PKG_VERSION).tar.gz
PKG_HASH:=1c729c1ea63100e878405cda7d66f9adfd3ed4f1d6cacaca0ee9152df728e016

PKG_LICENSE:=GPL-1.0-or-later Artistic-1.0-Perl
PKG_MAINTAINER:=Marcel Denia <naoir@gmx.net>

PKG_BUILD_DIR:=$(BUILD_DIR)/perl/HTTP-Negotiate-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include ../perl/perlmod.mk

define Package/perl-http-negotiate
  SUBMENU:=Perl
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Choose a variant to serve
  URL:=http://search.cpan.org/dist/HTTP-Negotiate/
  DEPENDS:=perl +perl-http-message +perlbase-essential
endef

define Build/Configure
        $(call perlmod/Configure,,)
endef

define Build/Compile
        $(call perlmod/Compile,,)
endef

define Package/perl-http-negotiate/install
        $(call perlmod/Install,$(1),HTTP auto/HTTP)
endef


$(eval $(call BuildPackage,perl-http-negotiate))
