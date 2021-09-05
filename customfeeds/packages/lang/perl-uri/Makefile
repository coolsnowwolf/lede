#
# Copyright (C) 2015 - 2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=perl-uri
PKG_VERSION:=1.76
PKG_RELEASE:=1

PKG_SOURCE:=URI-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://cpan.metacpan.org/authors/id/O/OA/OALDERS
PKG_HASH:=b2c98e1d50d6f572483ee538a6f4ccc8d9185f91f0073fd8af7390898254413e
PKG_BUILD_DIR:=$(BUILD_DIR)/perl/URI-$(PKG_VERSION)

PKG_MAINTAINER:=Marcel Denia <naoir@gmx.net>
PKG_LICENSE:=GPL-1.0-or-later Artistic-1.0-Perl
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk
include ../perl/perlmod.mk

define Package/perl-uri
  SUBMENU:=Perl
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Manipulates and accesses URI strings
  URL:=https://search.cpan.org/dist/URI/
  DEPENDS:=perl +perlbase-essential +perlbase-integer +perlbase-mime +perlbase-scalar +perlbase-utf8
endef

define Build/Configure
	$(call perlmod/Configure,,)
endef

define Build/Compile
	$(call perlmod/Compile,,)
endef

define Package/perl-uri/install
	$(call perlmod/Install,$(1),URI URI.pm)
endef


$(eval $(call BuildPackage,perl-uri))
