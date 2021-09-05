#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=perl-sub-uplevel
PKG_VERSION:=0.2800
PKG_RELEASE:=1

PKG_SOURCE:=Sub-Uplevel-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://cpan.metacpan.org/authors/id/D/DA/DAGOLDEN
PKG_HASH:=b4f3f63b80f680a421332d8851ddbe5a8e72fcaa74d5d1d98f3c8cc4a3ece293
PKG_BUILD_DIR:=$(BUILD_DIR)/perl/Sub-Uplevel-$(PKG_VERSION)

PKG_MAINTAINER:=Marcel Denia <naoir@gmx.net>
PKG_LICENSE:=GPL-1.0-or-later Artistic-1.0-Perl
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk
include ../perl/perlmod.mk

define Package/perl-sub-uplevel
  SUBMENU:=Perl
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Apparently run a function in a higher stack frame
  URL:=https://search.cpan.org/dist/Sub-Uplevel/
  DEPENDS:=perl +perlbase-essential
endef

define Build/Configure
        $(call perlmod/Configure,,)
endef

define Build/Compile
        $(call perlmod/Compile,,)
endef

define Package/perl-sub-uplevel/install
        $(call perlmod/Install,$(1),Sub auto/Sub)
endef


$(eval $(call BuildPackage,perl-sub-uplevel))
