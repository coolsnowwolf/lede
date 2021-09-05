#
# Copyright (C) 2014 - 2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=perl-test-harness
PKG_VERSION:=3.42
PKG_RELEASE:=1

PKG_SOURCE:=Test-Harness-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://cpan.metacpan.org/authors/id/L/LE/LEONT
PKG_HASH:=0fd90d4efea82d6e262e6933759e85d27cbcfa4091b14bf4042ae20bab528e53
PKG_BUILD_DIR:=$(BUILD_DIR)/perl/Test-Harness-$(PKG_VERSION)

PKG_MAINTAINER:=Marcel Denia <naoir@gmx.net>
PKG_LICENSE:=GPL-1.0-or-later Artistic-1.0-Perl

include $(INCLUDE_DIR)/package.mk
include ../perl/perlmod.mk

define Package/perl-test-harness
  SUBMENU:=Perl
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Perl Test Harness
  URL:=https://search.cpan.org/dist/Test-Harness/
  DEPENDS:=perl +perlbase-base +perlbase-benchmark +perlbase-config +perlbase-essential +perlbase-file +perlbase-getopt +perlbase-io +perlbase-posix +perlbase-text
endef

define Build/Configure
	$(call perlmod/Configure,,)
endef

define Build/Compile
	$(call perlmod/Compile,,)
endef

define Package/perl-test-harness/install
	$(call perlmod/Install,$(1),App auto/Test TAP Test)
endef


$(eval $(call BuildPackage,perl-test-harness))
