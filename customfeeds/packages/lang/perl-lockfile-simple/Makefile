#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=perl-lockfile-simple
PKG_VERSION:=0.208
PKG_RELEASE:=4

PKG_SOURCE_URL:=http://www.cpan.org/authors/id/S/SC/SCHWIGON/lockfile-simple/
PKG_SOURCE:=LockFile-Simple-$(PKG_VERSION).tar.gz
PKG_HASH:=45c77896b2a5a0a45f6202a6f813f437ff8b283f84a1c60d0c4f3730802af3a2

PKG_LICENSE:=GPL-2.0-or-later Artistic-1.0-Perl
PKG_MAINTAINER:=Marcel Denia <naoir@gmx.net>

PKG_BUILD_DIR:=$(BUILD_DIR)/perl/LockFile-Simple-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include ../perl/perlmod.mk

define Package/perl-lockfile-simple
  SUBMENU:=Perl
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Simple advisory file locking
  URL:=http://search.cpan.org/dist/LockFile-Simple/
  DEPENDS:=perl +perlbase-essential +perlbase-sys
endef

define Build/Configure
	$(call perlmod/Configure,,)
endef

define Build/Compile
	$(call perlmod/Compile,,)
endef

define Package/perl-lockfile-simple/install
	$(call perlmod/Install,$(1),LockFile auto/LockFile)
endef


$(eval $(call BuildPackage,perl-lockfile-simple))
