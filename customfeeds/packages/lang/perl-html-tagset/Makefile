#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=perl-html-tagset
PKG_VERSION:=3.20
PKG_RELEASE:=4

PKG_SOURCE_URL:=http://www.cpan.org/authors/id/P/PE/PETDANCE/
PKG_SOURCE:=HTML-Tagset-$(PKG_VERSION).tar.gz
PKG_HASH:=adb17dac9e36cd011f5243881c9739417fd102fce760f8de4e9be4c7131108e2

PKG_LICENSE:=GPL-1.0-or-later Artistic-1.0-Perl
PKG_MAINTAINER:=Marcel Denia <naoir@gmx.net>

PKG_BUILD_DIR:=$(BUILD_DIR)/perl/HTML-Tagset-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include ../perl/perlmod.mk

define Package/perl-html-tagset
  SUBMENU:=Perl
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Data tables pertaining to HTML
  URL:=http://search.cpan.org/dist/HTML-Tagset/
  DEPENDS:=perl +perlbase-essential
endef

define Build/Configure
	$(call perlmod/Configure,,)
endef

define Build/Compile
	$(call perlmod/Compile,,)
endef

define Package/perl-html-tagset/install
	$(call perlmod/Install,$(1),HTML)
endef


$(eval $(call BuildPackage,perl-html-tagset))
