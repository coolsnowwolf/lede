#
# Copyright (C) 2010-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=perl-www-mechanize
PKG_VERSION:=1.96
PKG_RELEASE:=1

PKG_SOURCE:=WWW-Mechanize-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://cpan.metacpan.org/authors/id/O/OA/OALDERS
PKG_HASH:=a79a613452287433a88e689195b09951a06c2df6d7fd40c15aa556452de9ab04
PKG_BUILD_DIR:=$(BUILD_DIR)/perl/WWW-Mechanize-$(PKG_VERSION)

PKG_MAINTAINER:=Marcel Denia <naoir@gmx.net>
PKG_LICENSE:=GPL-1.0-or-later Artistic-1.0-Perl
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk
include ../perl/perlmod.mk

define Package/perl-www-mechanize
  SUBMENU:=Perl
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Perl WWW Mechanize
  URL:=https://search.cpan.org/dist/WWW-Mechanize/
  DEPENDS:=perl +perl-cgi +perl-html-form +perl-html-parser +perl-html-tree +perl-http-daemon +perl-http-message +perl-http-server-simple +perl-test-warn +perl-uri +perl-www +perlbase-base +perlbase-essential +perlbase-file +perlbase-findbin +perlbase-getopt +perlbase-pod +perlbase-test
endef

define Build/Configure
	$(call perlmod/Configure,$(STAGING_DIR)/usr/include,)
endef

define Build/Compile
	$(call perlmod/Compile,,)
endef

define Package/perl-www-mechanize/install
	$(call perlmod/Install,$(1),WWW/Mechanize WWW/Mechanize.pm)
endef


$(eval $(call BuildPackage,perl-www-mechanize))
