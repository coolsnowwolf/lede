#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=perl-html-tree
PKG_VERSION:=3.23
PKG_RELEASE:=5

PKG_SOURCE_URL:=http://www.cpan.org/authors/id/P/PE/PETEK/
PKG_SOURCE:=HTML-Tree-$(PKG_VERSION).tar.gz
PKG_HASH:=f5175acf262f3710dce899796ea3e353049939400b100706d03df2f08803c8de

PKG_LICENSE:=GPL-1.0-or-later Artistic-1.0-Perl
PKG_MAINTAINER:=Marcel Denia <naoir@gmx.net>

PKG_BUILD_DIR:=$(BUILD_DIR)/perl/HTML-Tree-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include ../perl/perlmod.mk

define Package/perl-html-tree
  SUBMENU:=Perl
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Represent and create HTML syntax trees
  URL:=http://search.cpan.org/dist/HTML-Tree/
  DEPENDS:=perl +perl-html-parser +perl-html-tagset +perlbase-essential +perlbase-integer
endef

define Build/Configure
	$(call perlmod/Configure,,)
endef

define Build/Compile
	$(call perlmod/Compile,,)
endef

define Package/perl-html-tree/install
	$(call perlmod/Install,$(1),HTML)
endef


$(eval $(call BuildPackage,perl-html-tree))
