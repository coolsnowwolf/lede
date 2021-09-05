#
# Copyright (C) 2020 Philip Prindeville, Redfish Solutions, LLC
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=perl-net-cidr-lite
PKG_VERSION:=0.21
PKG_RELEASE:=1

PKG_SOURCE:=Net-CIDR-Lite-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://cpan.metacpan.org/authors/id/D/DO/DOUGW
PKG_HASH:=cfa125e8a2aef9259bc3a44e07cbdfb7894b64d22e7c0cee92aee2f5c7915093
PKG_BUILD_DIR:=$(BUILD_DIR)/perl/Net-CIDR-Lite-$(PKG_VERSION)

PKG_MAINTAINER:=Philip Prindeville <philipp@redfish-solutions.com>
PKG_LICENSE:=GPL-1.0-or-later Artistic-1.0-Perl

include $(INCLUDE_DIR)/package.mk
include ../perl/perlmod.mk

define Package/perl-net-cidr-lite
  SUBMENU:=Perl
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Perl extension for merging IPv4 or IPv6 CIDR addresses
  URL:=https://search.cpan.org/dist/Net-CIDR-Lite/
  DEPENDS:=perl +perlbase-essential +perlbase-config +perlbase-io +perlbase-dynaloader
  PKGARCH:=all
endef

define Build/Configure
        $(call perlmod/Configure,,)
endef

define Build/Compile
        $(call perlmod/Compile,,)
endef

define Package/perl-net-cidr-lite/install
        $(call perlmod/Install,$(1),Net Net/CIDR Net/CIDR/Lite.pm)
endef


$(eval $(call BuildPackage,perl-net-cidr-lite))
