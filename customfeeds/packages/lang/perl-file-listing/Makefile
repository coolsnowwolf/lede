#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=perl-file-listing
PKG_VERSION:=6.04
PKG_RELEASE:=2

PKG_SOURCE_URL:=http://www.cpan.org/authors/id/G/GA/GAAS
PKG_SOURCE:=File-Listing-$(PKG_VERSION).tar.gz
PKG_HASH:=1e0050fcd6789a2179ec0db282bf1e90fb92be35d1171588bd9c47d52d959cf5

PKG_LICENSE:=GPL-1.0-or-later Artistic-1.0-Perl
PKG_MAINTAINER:=Marcel Denia <naoir@gmx.net>

PKG_BUILD_DIR:=$(BUILD_DIR)/perl/File-Listing-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include ../perl/perlmod.mk

define Package/perl-file-listing
  SUBMENU:=Perl
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Parse directory listing
  URL:=http://search.cpan.org/dist/File-Listing/
  DEPENDS:=perl +perl-http-date +perlbase-essential
endef

define Build/Configure
        $(call perlmod/Configure,,)
endef

define Build/Compile
        $(call perlmod/Compile,,)
endef

define Package/perl-file-listing/install
        $(call perlmod/Install,$(1),File auto/File)
endef


$(eval $(call BuildPackage,perl-file-listing))
