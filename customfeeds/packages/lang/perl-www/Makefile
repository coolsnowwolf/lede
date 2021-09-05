#
# Copyright (C) 2013-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=perl-www
PKG_VERSION:=6.43
PKG_RELEASE:=2

PKG_SOURCE_URL:=https://cpan.metacpan.org/authors/id/O/OA/OALDERS
PKG_SOURCE:=libwww-perl-$(PKG_VERSION).tar.gz
PKG_HASH:=e9849d7ee6fd0e89cc999e63d7612c951afd6aeea6bc721b767870d9df4ac40d
PKG_BUILD_DIR:=$(BUILD_DIR)/perl/libwww-perl-$(PKG_VERSION)

PKG_MAINTAINER:=Marcel Denia <naoir@gmx.net>
PKG_LICENSE:=GPL-1.0-or-later Artistic-1.0-Perl
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID:=cpe:/a:search.cpan:libwww-perl

include $(INCLUDE_DIR)/package.mk
include ../perl/perlmod.mk

define Package/perl-www
  SUBMENU:=Perl
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=WWW client/server library for Perl (aka LWP)
  URL:=https://search.cpan.org/dist/libwww-perl/
  DEPENDS:=perl +perl-encode-locale +perl-file-listing +perl-html-parser +perl-http-cookies +perl-http-daemon +perl-http-date +perl-http-message +perl-http-negotiate +perl-lwp-mediatypes +perl-net-http +perl-try-tiny +perl-uri +perl-www-robotrules +perlbase-base +perlbase-digest +perlbase-encode +perlbase-essential +perlbase-io +perlbase-mime +perlbase-net
endef

define Build/Configure
	$(call perlmod/Configure,,)
endef

define Build/Compile
	$(call perlmod/Compile,,)
endef

define Package/perl-www/install
	$(call perlmod/Install,$(1),LWP.pm LWP LWP)
endef


$(eval $(call BuildPackage,perl-www))
