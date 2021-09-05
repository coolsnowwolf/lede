#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=perl-lwp-mediatypes
PKG_VERSION:=6.04
PKG_RELEASE:=1

PKG_SOURCE:=LWP-MediaTypes-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://cpan.metacpan.org/authors/id/O/OA/OALDERS
PKG_HASH:=8f1bca12dab16a1c2a7c03a49c5e58cce41a6fec9519f0aadfba8dad997919d9
PKG_BUILD_DIR:=$(BUILD_DIR)/perl/LWP-MediaTypes-$(PKG_VERSION)

PKG_MAINTAINER:=Marcel Denia <naoir@gmx.net>
PKG_LICENSE:=GPL-1.0-or-later Artistic-1.0-Perl
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk
include ../perl/perlmod.mk

define Package/perl-lwp-mediatypes
  SUBMENU:=Perl
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Guess media type for a file or a URL
  URL:=https://search.cpan.org/dist/LWP-MediaTypes/
  DEPENDS:=perl +perlbase-essential
endef

define Build/Configure
        $(call perlmod/Configure,,)
endef

define Build/Compile
        $(call perlmod/Compile,,)
endef

define Package/perl-lwp-mediatypes/install
        $(call perlmod/Install,$(1),LWP auto/LWP)
endef


$(eval $(call BuildPackage,perl-lwp-mediatypes))
