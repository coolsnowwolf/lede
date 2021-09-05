#
# Copyright (C) 2014, 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=perl-compress-bzip2
PKG_VERSION:=2.26
PKG_RELEASE:=1

PKG_SOURCE:=Compress-Bzip2-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://www.cpan.org/authors/id/R/RU/RURBAN/
PKG_HASH:=5132f0c5f377a54d77ee36d332aa0ece585c22a40f2c31f2619e40262f5c4f0c
PKG_BUILD_DIR:=$(BUILD_DIR)/perl/Compress-Bzip2-$(PKG_VERSION)

PKG_MAINTAINER:=Marcel Denia <naoir@gmx.net>
PKG_LICENSE:=GPL-1.0-or-later Artistic-1.0-Perl
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk
include ../perl/perlmod.mk

define Package/perl-compress-bzip2
  SUBMENU:=Perl
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Perl interface to bzip2 compression library
  URL:=https://search.cpan.org/dist/Compress-Bzip2/
  DEPENDS:=perl +libbz2 +perlbase-autoloader +perlbase-config +perlbase-essential +perlbase-fcntl +perlbase-file +perlbase-getopt +perlbase-test +perlbase-xsloader
endef

define Build/Configure
	$(call perlmod/Configure,,BUILD_BZLIB=0)
endef

define Build/Compile
	$(call perlmod/Compile,,)
endef

define Package/perl-compress-bzip2/install
	$(call perlmod/Install,$(1),Compress auto/Compress)
endef


$(eval $(call BuildPackage,perl-compress-bzip2))
