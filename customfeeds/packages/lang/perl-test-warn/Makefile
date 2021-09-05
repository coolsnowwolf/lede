#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=perl-test-warn
PKG_VERSION:=0.36
PKG_RELEASE:=1

PKG_SOURCE:=Test-Warn-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://cpan.metacpan.org/authors/id/B/BI/BIGJ
PKG_HASH:=ecbca346d379cef8d3c0e4ac0c8eb3b2613d737ffaaeae52271c38d7bf3c6cda
PKG_BUILD_DIR:=$(BUILD_DIR)/perl/Test-Warn-$(PKG_VERSION)

PKG_MAINTAINER:=Marcel Denia <naoir@gmx.net>
PKG_LICENSE:=GPL-1.0-or-later Artistic-1.0-Perl

include $(INCLUDE_DIR)/package.mk
include ../perl/perlmod.mk

define Package/perl-test-warn
  SUBMENU:=Perl
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Perl extension to test methods for warnings
  URL:=https://search.cpan.org/dist/Test-Warn/
  DEPENDS:=perl +perl-sub-uplevel +perlbase-essential +perlbase-test
endef

define Build/Configure
        $(call perlmod/Configure,,)
endef

define Build/Compile
        $(call perlmod/Compile,,)
endef

define Package/perl-test-warn/install
        $(call perlmod/Install,$(1),Test auto/Test)
endef


$(eval $(call BuildPackage,perl-test-warn))
