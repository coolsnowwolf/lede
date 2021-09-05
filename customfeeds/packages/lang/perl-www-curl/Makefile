#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=perl-www-curl
PKG_VERSION:=4.17
PKG_RELEASE:=7

PKG_SOURCE_URL:=http://www.cpan.org/authors/id/S/SZ/SZBALINT/
PKG_SOURCE:=WWW-Curl-$(PKG_VERSION).tar.gz
PKG_HASH:=52ffab110e32348d775f241c973eb56f96b08eedbc110d77d257cdb0a24ab7ba

PKG_LICENSE:=GPL-1.0-or-later Artistic-1.0-Perl
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Marcel Denia <naoir@gmx.net>

PKG_BUILD_DIR:=$(BUILD_DIR)/perl/WWW-Curl-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include ../perl/perlmod.mk

define Package/perl-www-curl
  SUBMENU:=Perl
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Perl bindings to libcurl
  URL:=http://search.cpan.org/dist/WWW-Curl/
  DEPENDS:=perl +libcurl +perlbase-essential +perlbase-xsloader
endef

define Build/Configure
	$(call perlmod/Configure,$(STAGING_DIR)/usr/include,)
endef

define Build/Compile
	$(call perlmod/Compile,,)
endef

define Package/perl-www-curl/install
	$(call perlmod/Install,$(1),WWW/Curl WWW/Curl.pm auto/WWW/Curl)
endef


$(eval $(call BuildPackage,perl-www-curl))
