#
# Copyright (C) 2017 Philip Prindeville <philipp@redfish-solutions.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=perl-authen-sasl
PKG_VERSION:=2.16
PKG_RELEASE:=2

PKG_SOURCE_NAME:=Authen-SASL
PKG_SOURCE_URL:=http://www.cpan.org/authors/id/G/GB/GBARR/
PKG_SOURCE:=$(PKG_SOURCE_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=6614fa7518f094f853741b63c73f3627168c5d3aca89b1d02b1016dc32854e09

PKG_LICENSE:=GPL-1.0-or-later Artistic-1.0-Perl
PKG_MAINTAINER:=Philip Prindeville <philipp@redfish-solutions.com>

PKG_BUILD_DIR:=$(BUILD_DIR)/perl/$(PKG_SOURCE_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include ../perl/perlmod.mk

define Package/perl-authen-sasl
  SUBMENU:=Perl
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Generic network authentication protocol framework.
  URL:=http://search.cpan.org/dist/$(PKG_SOURCE_NAME)/
  DEPENDS:=perl +perlbase-digest
endef

define Build/Configure
	$(call perlmod/Configure,,)
endef

define Build/Compile
	$(call perlmod/Compile,,)
endef

define Package/perl-authen-sasl/install
	$(call perlmod/Install,$(1),Authen)
endef


$(eval $(call BuildPackage,perl-authen-sasl))
